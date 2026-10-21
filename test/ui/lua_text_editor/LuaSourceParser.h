#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include "TreeCursor.h"
#include "LuaSourceParserCommon.h"
#include "../../core/Result.h"
#include "../../core/commonObjects.h"

namespace ui {

using UniqueTSParserPtr = std::unique_ptr<TSParser, 
	decltype([](TSParser* p) { ts_parser_delete(p); })>;

inline UniqueTSParserPtr CreateUniqueTSParserPtr()
{
    return UniqueTSParserPtr(ts_parser_new());
}

using UniqueTSTreePtr = std::unique_ptr<TSTree, 
	decltype([](TSTree* t) { ts_tree_delete(t); })>;

inline UniqueTSTreePtr CreateUniqueTSTreePtr(TSParser* parser, const TSTree* oldTree, 
											 std::string_view src)
{
	const auto size = static_cast<uint32_t>(src.size());
	return UniqueTSTreePtr(ts_parser_parse_string(parser, oldTree, src.data(), size));
}

inline UniqueTSTreePtr CreateUniqueTSTreePtr(TSParser* parser, const TSTree* oldTree,
											 TSInput input)
{
	return UniqueTSTreePtr(ts_parser_parse(parser, oldTree, input));
}

struct SourceLocationDataIndexPair
{
	SourceLocation location;
	size_t dataIndex = std::numeric_limits<size_t>::max();

	friend constexpr bool operator<(const SourceLocationDataIndexPair& lhs, 
									const SourceLocationDataIndexPair& rhs) noexcept
	{
		return lhs.location.start < rhs.location.start;
	}
};

struct ParsedLuaSourceDocument
{
	std::string_view functionTableName;
	std::string_view metaTableName;
	std::unordered_map<std::string_view, size_t> functionDataIndex;
	std::vector<SourceLocationDataIndexPair> functionBlockInfo;
	std::vector<std::string_view> functionNames;
	std::vector<std::vector<std::string_view>> functionArgumentTypes;
	std::vector<std::vector<std::string_view>> functionParameterNames;
	std::vector<SourceDocumentError> errors;

	bool HasErrors() const noexcept { return !errors.empty(); }

	void Print() const;
};

class LuaSourceParser
{
public:
    LuaSourceParser();
	~LuaSourceParser() = default;
	LuaSourceParser(const LuaSourceParser&) = delete;
	LuaSourceParser& operator=(const LuaSourceParser&) = delete;
	LuaSourceParser(LuaSourceParser&&) noexcept = default;
	LuaSourceParser& operator=(LuaSourceParser&&) noexcept = default;

	bool HasErrors() const;

	void Parse(std::string source);

	void EditSource(std::string&& newSource);

	void Reset();

	TreeNode GetRootNode() const;
	TreeNode GetNodeAtByteOffset(size_t byteOffset) const;
	TreeNode GetNodeAtPoint(TSPoint point);

	const std::string& GetSourceString() const { return source_; }
	const ParsedLuaSourceDocument& GetSourceDocument() const { return sourceDocument_; }

	void PrintSyntaxTree();

	bool AddCompletionMarker(size_t markerSourceOffset);
	bool RemoveCompletionMarker(size_t markerSourceOffset);
	void ClearCompletionMarkers();

	TreeCursor GetCursorAtRoot() const { return TreeCursor(GetRootNode(), source_); }
	TreeCursor GetCursorAtNode(const TreeNode& node) const { return TreeCursor(node, source_); }

private:
	ParsedLuaSourceDocument ParseDocument();
	TreeCursor GetCursorAtByteOffset(size_t byteOffset) const;

	static void PrintSyntaxTreeImpl(TSNode node, std::string_view source, int depth = 0);

	UniqueTSParserPtr parser_;
	UniqueTSTreePtr tree_;
	std::string source_;
	ParsedLuaSourceDocument sourceDocument_;
	std::vector<size_t> completionMarkerOffsets_;
};

} // ui

#endif