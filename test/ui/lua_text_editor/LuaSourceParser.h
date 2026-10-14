#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include "../../core/Result.h"
#include "../../core/commonObjects.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../../deps/tree-sitter-master/lib/include/tree_sitter/api.h"
    const TSLanguage* tree_sitter_lua();
#ifdef __cplusplus
}
#endif

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

struct TreeNode
{
	TreeNode() : node{0} {}
	TreeNode(TSNode node, std::string_view src) : node(node), source(src) {}

	operator TSNode& () { return node; }
	operator const TSNode& () const { return node; }

	operator bool() const { return !ts_node_is_null(node); }
	bool operator!() const { return ts_node_is_null(node); }

	bool HasErrors() const { return ts_node_has_error(node); }
	bool IsMissing() const { return ts_node_is_missing(node); }
	bool IsNamed() const { return ts_node_is_named(node); }

	size_t GetChildCount() const { return ts_node_child_count(node); }
	size_t GetNamedChildCount() const { return ts_node_named_child_count(node); }

	std::string_view GetText() const;
	std::string_view GetType() const;

	size_t GetStartPos() const { return ts_node_start_byte(node); }
	size_t GetEndPos() const { return ts_node_end_byte(node); }
	TSPoint GetStartPoint() const { return ts_node_start_point(node); }
	TSPoint GetEndPoint() const { return ts_node_end_point(node); }
	
	TSNode node;
	std::string_view source;
};

struct TreeCursor
{
	TreeCursor() : cursor{}, source{} {}
	TreeCursor(TSNode node, std::string_view src) : cursor(ts_tree_cursor_new(node)), source(src) {}
	~TreeCursor() { ts_tree_cursor_delete(&cursor); }
	TreeCursor(const TreeCursor&) = delete;
	TreeCursor& operator=(const TreeCursor&) = delete;
	TreeCursor(TreeCursor&& other) noexcept : cursor(other.cursor), source(other.source)
	{
		other.cursor = {};
	}
	TreeCursor& operator=(TreeCursor&& other) noexcept
	{
		if (this == &other) { return *this; }

		cursor = other.cursor;
		source = other.source;
		other.cursor = {};
		return *this;
	}

	operator bool() const { return !source.empty(); }
	bool operator!() const { return source.empty(); }

	TreeNode CurrentNode() const { return TreeNode(ts_tree_cursor_current_node(&cursor), source); }

	bool GoToFirstChild() { return ts_tree_cursor_goto_first_child(&cursor); }
	bool GoToNextSibling() { return ts_tree_cursor_goto_next_sibling(&cursor); }
	bool GoToPrevSibling() { return ts_tree_cursor_goto_previous_sibling(&cursor); }
	bool GoToParent() { return ts_tree_cursor_goto_parent(&cursor); }
	bool GoToParent(size_t lvls);
	void Reset() { while (GoToParent()) {} }
	bool GoToFirstNamedChild();
	bool GoToNextNamedSibling();

	TSTreeCursor cursor;
	std::string_view source;
};

struct SourceLocation
{
	size_t start = 0;
	size_t end = 0;
};

struct SourceDocumentError
{
	std::string message;
	SourceLocation location;
};

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
	//static constexpr std::string_view kMethodIndexCompletionMarker = "__method_completion_marker__()";
	//static constexpr std::string_view kDotIndexCompletionMarker = "__dot_completion_marker__";
	//static constexpr std::string_view kCompletionMarkerText = "__completion_marker__";

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

	std::optional<size_t> GetEnclosingFunctionDataIndex(size_t byteOffset) const;

	TreeNode GetNodeAtByteOffset(size_t byteOffset) const;

	TreeNode GetNodeAtPoint(TSPoint point);

	const std::string& GetSourceString() const { return source_; }
	const ParsedLuaSourceDocument& GetSourceDocument() const { return sourceDocument_; }

	void PrintSyntaxTree();

	bool AddCompletionMarker(size_t markerSourceOffset);
	bool RemoveCompletionMarker(size_t markerSourceOffset);
	void ClearCompletionMarkers();

private:
	TreeNode GetRootNode() const;
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