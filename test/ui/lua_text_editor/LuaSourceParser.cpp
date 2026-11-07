#include "LuaSourceParser.h"

#if IMGUI_ENABLED
#include <cassert>
#include <unordered_map>
#include <ranges>
#include <span>
#include "TSEditorBridge.h"
#include "../../core/Logger.h"

namespace ui {

namespace {

std::optional<SourceLocation> MapOldFunctionBlockLocationToNew(const SourceLocation& loc, const TSInputEdit& edit)
{
    const auto delta = static_cast<ptrdiff_t>(edit.new_end_byte) - static_cast<ptrdiff_t>(edit.old_end_byte);
    auto shift = [delta](size_t v) { return static_cast<size_t>(static_cast<ptrdiff_t>(v) + delta); };

    if (loc.end <= edit.start_byte)
    {
        return loc; // before the edit
    }
    if (loc.start >= edit.old_end_byte)
    {
        return SourceLocation{ shift(loc.start), shift(loc.end) }; // after the edit
    }
    if (loc.start < edit.start_byte && loc.end > edit.old_end_byte)
    {
        return SourceLocation{ loc.start, shift(loc.end) }; // edit is inside this block
    }

    return std::nullopt; // partial overlap: can't trust it
}

bool LocationInsideEditedPortion(const SourceLocation& loc, const TSInputEdit& edit)
{
    assert(edit.start_byte >= 0);
    assert(edit.old_end_byte >= 0);

    return loc.end > static_cast<size_t>(edit.start_byte) && 
           loc.start < static_cast<size_t>(edit.old_end_byte);
}

template <typename T, typename U>
bool FunctionBlocksEqual(const FunctionBlockInfoTemplate<T>& lhs,
    const FunctionBlockInfoTemplate<U>& rhs)
{
    auto sameStrings = [](const auto& a, const auto& b) {
        return std::ranges::equal(a, b, [](const auto& x, const auto& y) {
            return std::string_view{ x } == std::string_view{ y };
        });
     };

    return lhs.location.start == rhs.location.start &&
           lhs.location.end == rhs.location.end &&
           lhs.functionName == rhs.functionName &&
           sameStrings(lhs.parameterNames, rhs.parameterNames) &&
           sameStrings(lhs.parameterTypes, rhs.parameterTypes);
}

template <typename T, typename U>
bool FunctionBlocksEqual(const std::vector<FunctionBlockInfoTemplate<T>>& lhs,
    const std::vector<FunctionBlockInfoTemplate<U>>& rhs)
{
    return std::ranges::equal(lhs, rhs, [](const auto& a, const auto& b) {
        return FunctionBlocksEqual(a, b);
    });
}

FunctionBlockInfoTemplate<std::string_view>
MakeFunctionBlockView(const FunctionBlockInfoTemplate<std::string>& block)
{
    return FunctionBlockInfoTemplate<std::string_view>{
        .location = block.location,
        .functionName = block.functionName,
        .parameterNames = block.parameterNames
            | std::views::transform([](const auto& s) { return std::string_view{ s }; })
            | std::ranges::to<std::vector>(),
        .parameterTypes = block.parameterTypes
            | std::views::transform([](const auto& s) { return std::string_view{ s }; })
            | std::ranges::to<std::vector>()
        };
};

FunctionBlockInfoTemplate<std::string>
MakeBackupBlock(const FunctionBlockInfoTemplate<std::string_view>& block)
{
    return FunctionBlockInfoTemplate<std::string>{
        .location = block.location,
            .functionName = std::string{ block.functionName },
            .parameterNames = block.parameterNames
            | std::views::transform([](const auto& s) { return std::string{ s }; })
            | std::ranges::to<std::vector>(),
            .parameterTypes = block.parameterTypes
            | std::views::transform([](const auto& s) { return std::string{ s }; })
            | std::ranges::to<std::vector>()
    };
};


std::pair<Range<size_t>, Range<size_t>> 
GetEditedFunctionBlockRanges(std::vector<FunctionBlockInfoTemplate<std::string>>& backupBlocks,
                             const std::vector<FunctionBlockInfoTemplate<std::string_view>>& newBlocks,
                             const TSInputEdit& tsEdit)
{
    auto result = std::make_pair(
        Range<size_t>{ std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::min() },
        Range<size_t>{ std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::min() }
    );

	auto& [backupBlockIndices, newBlockIndices] = result;
    
	const size_t size = std::max(backupBlocks.size(), newBlocks.size());
    const auto delta = tsEdit.new_end_byte - tsEdit.old_end_byte;

    for (size_t i = 0; i < size; ++i)
    {
        if (i < backupBlocks.size())
        {
            auto& backupBlock = backupBlocks[i];
            if (LocationInsideEditedPortion(backupBlock.location, tsEdit))
            {
                backupBlockIndices.min = std::min(backupBlockIndices.min, i);
                backupBlockIndices.max = std::max(backupBlockIndices.max, i);

                backupBlock.location.start += delta;
                backupBlock.location.end += delta;
            }
		}
        if (i < newBlocks.size())
        {
            const auto& newBlock = newBlocks[i];
            if (LocationInsideEditedPortion(newBlock.location, tsEdit))
            {
                newBlockIndices.min = std::min(newBlockIndices.min, i);
                newBlockIndices.max = std::max(newBlockIndices.max, i);
            }
		}
    }

    return result;
}

} // unnamed

LuaSourceParser::LuaSourceParser() : parser_(CreateUniqueTSParserPtr()), tree_(nullptr)
{
    const auto* language = tree_sitter_lua();

    const bool success = ts_parser_set_language(parser_.get(), language);

    assert(success);
}

void LuaSourceParser::Parse(std::string source)
{
    Reset();

    tree_ = CreateUniqueTSTreePtr(
        parser_.get(),
        nullptr,
        source);

	source_ = std::move(source);

    auto root = GetCursorAtRoot();
    sourceDocument_ = LuaSourceDocument::Parse(root);
}

void LuaSourceParser::Parse(std::string source, const ParsedLuaUserTypeInfo& userTypeInfo)
{
    Reset();

    tree_ = CreateUniqueTSTreePtr(
        parser_.get(),
        nullptr,
        source);

    source_ = std::move(source);

    auto root = GetCursorAtRoot();
    sourceDocument_ = LuaSourceDocument::Parse(root, userTypeInfo);
}

LuaSourceDocument LuaSourceParser::ParseDocument()
{
    if (!tree_ || source_.empty())
    {
        return {};
    }

    auto cursor = GetCursorAtRoot();

	return LuaSourceDocument::Parse(cursor);
}

TreeCursor LuaSourceParser::GetCursorAtByteOffset(size_t byteOffset) const
{
    if (!tree_ || byteOffset < source_.size())
    {
        return {};
    }

    return TreeCursor(ts_node_descendant_for_byte_range(
        GetRootNode(),
        byteOffset,
        byteOffset), source_);
}

//void LuaSourceParser::EditSource(std::string&& newSource)
//{
//    if (!tree_ || source_.empty())
//    {
//        return;
//    }
//
//    auto tsEdit = MakeTSInputEdit(source_, newSource);
//
//    ts_tree_edit(tree_.get(), &tsEdit);
//
//    VirtualSource virtualSource{ .source = newSource };
//	virtualSource.PrepareOffsets(completionMarkerOffsets_);
//
//    TSInput input{
//        .payload = &virtualSource,
//        .read = &VirtualSource::Read,
//        .encoding = TSInputEncodingUTF8,
//        .decode = nullptr
//    };
//
//    tree_ = CreateUniqueTSTreePtr(parser_.get(), nullptr, input);
//
//    if (!completionMarkerOffsets_.empty())
//    {
//        LOG_WARNING("\n======================================================================");
//
//        LOG_INFO("--- SEGMENTS ---");
//        LOG_DEBUG_FMT("count: '{}'", virtualSource.segments.size());
//        for (size_t i = 0; i < virtualSource.segments.size(); ++i)
//        {
//			const auto& seg = virtualSource.segments[i];
//            LOG_DEBUG_FMT("virtualStart: '{}', length: '{}', data: '{}'",
//                seg.virtualStart, seg.length, seg.data + seg.virtualStart);
//        }
//
//        LOG_INFO("\n--- READ CALLBACK RETURNS ---");
//        for (size_t i = 0; i < virtualSource.test.size(); ++i)
//        {
//			LOG_DEBUG_FMT("[{}] '{}'", i, virtualSource.test[i]);
//        }
//        LOG_WARNING("======================================================================\n");
//    }
//
//  //  tree_ = CreateUniqueTSTreePtr(
//  //      parser_.get(),
//  //      tree_.get(),
//		//newSource);
//
//	source_ = std::move(newSource);
//    sourceDocument_ = ParseDocument();
//}

void LuaSourceParser::EditSource(std::string&& newSource)
{
    if (!tree_ || source_.empty())
    {
        return;
    }

    auto tsEdit = MakeTSInputEdit(source_, newSource);

    ts_tree_edit(tree_.get(), &tsEdit);

    tree_ = CreateUniqueTSTreePtr(
          parser_.get(),
          tree_.get(),
          newSource);

    source_ = std::move(newSource);

    auto cursor = GetCursorAtRoot();
    sourceDocument_ = LuaSourceDocument::Parse(cursor);

  //  if (HasErrors())
  //  {
		//ApplyBackupBlocks(tsEdit);
  //  }
  //  else
  //  {
  //      RefreshBackup();
  //  }
}

void LuaSourceParser::EditSource(std::string&& newSource, const ParsedLuaUserTypeInfo& userTypeInfo)
{
    if (!tree_ || source_.empty())
    {
        return;
    }

    auto tsEdit = MakeTSInputEdit(source_, newSource);

    ts_tree_edit(tree_.get(), &tsEdit);

    tree_ = CreateUniqueTSTreePtr(
        parser_.get(),
        tree_.get(),
        newSource);

    source_ = std::move(newSource);

    auto cursor = GetCursorAtRoot();
    sourceDocument_ = LuaSourceDocument::Parse(cursor, userTypeInfo);

    if (HasErrors())
    {
        ApplyBackupBlocks(tsEdit);
    }
    else
    {
        RefreshBackup();
    }
}

void LuaSourceParser::RefreshBackup()
{
    backup_.functionBlocks.clear();

    for (const auto& block : sourceDocument_.functionBlocks)
    {
        auto& backupBlock = backup_.functionBlocks.emplace_back();

        backupBlock.location = block.location;
        backupBlock.functionName = std::string{ block.functionName };
        for (const auto& n : block.parameterNames) backupBlock.parameterNames.emplace_back(n);
        for (const auto& t : block.parameterTypes) backupBlock.parameterTypes.emplace_back(t);
    }
}

void LuaSourceParser::ApplyBackupBlocks(const TSInputEdit& edit)
{
    if (backup_.functionBlocks.empty())
    {
        return;
    }

    auto& blocks = sourceDocument_.functionBlocks;

    for (const auto& old : backup_.functionBlocks)
    {
        const auto mapped = MapOldFunctionBlockLocationToNew(old.location, edit);
        if (!mapped)
        {
            continue;
        }

        auto match = std::ranges::find_if(blocks, [&](const auto& f) {
            return f.functionName == old.functionName;
        });

        if (match != blocks.end() && match->location == mapped)
        {
            continue; // fresh parse agrees with the backup
        }

        // remove the wrong/swallowing fresh block(s)
        std::erase_if(blocks, [&](const auto& f) {
            return f.functionName == old.functionName ||
                   f.location.Overlaps(*mapped);
        });

        auto& restored = blocks.emplace_back();

        restored.location = *mapped;
        restored.functionName = old.functionName;
        for (const auto& n : old.parameterNames) restored.parameterNames.emplace_back(n);
        for (const auto& t : old.parameterTypes) restored.parameterTypes.emplace_back(t);
    }

    std::ranges::sort(blocks, {}, [](const auto& b) { return b.location.start; });
}

bool LuaSourceParser::HasErrors() const
{
    if (!tree_ || source_.empty())
    {
        return false;
    }

    return ts_node_has_error(ts_tree_root_node(tree_.get()));
}

void LuaSourceParser::Reset()
{
    tree_.reset();
    source_ = {};
	sourceDocument_ = {};
	backup_ = {};
}

TreeNode LuaSourceParser::GetNodeAtByteOffset(size_t byteOffset) const
{
    if (!tree_ || source_.empty())
    {
        return {};
	}

    return TreeNode(ts_node_descendant_for_byte_range(
        GetRootNode(),
        byteOffset,
        byteOffset), source_);
}

TreeNode LuaSourceParser::GetNodeAtPoint(TSPoint point)
{
    return TreeNode(ts_node_descendant_for_point_range(
            GetRootNode(),
            point,
            point), source_);
}

TreeNode LuaSourceParser::GetRootNode() const
{
    return TreeNode(ts_tree_root_node(tree_.get()), source_);
}

void LuaSourceParser::PrintSyntaxTree()
{
    if (!tree_ || source_.empty())
    {
        std::cout << "No syntax tree to print.\n";
        return;
    }

	PrintSyntaxTreeImpl(ts_tree_root_node(tree_.get()), source_, 0);
}

void LuaSourceParser::PrintSyntaxTreeImpl(TSNode node, std::string_view source, int depth)
{
    const char* type = ts_node_type(node);

    const uint32_t start = ts_node_start_byte(node);
    const uint32_t end = ts_node_end_byte(node);

    std::cout << std::string(depth * 2, '-')
        << type
        << " [" << start << ", " << end << "]";

    if (end >= start && end <= source.size())
    {
        std::cout << " `"
            << source.substr(start, end - start)
            << '`';
    }

    if (ts_node_is_error(node))
        std::cout << " [ERROR]";

    if (ts_node_is_missing(node))
        std::cout << " [MISSING]";

    std::cout << '\n';

    const uint32_t count = ts_node_child_count(node);

    for (uint32_t i = 0; i < count; ++i)
    {
        PrintSyntaxTreeImpl(
            ts_node_child(node, i),
            source,
            depth + 1
        );
    }
}

} // ui

#endif