#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <string_view>
#include <vector>
#include <format>
#include "LuaSourceParserCommon.h"
#include "../../core/FixedString.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../../deps/tree-sitter-master/lib/include/tree_sitter/api.h"
    const TSLanguage* tree_sitter_lua();
#ifdef __cplusplus
}
#endif

namespace ui {

struct TreeNode
{
	TreeNode() : node{ 0 } {}
	TreeNode(TSNode node, std::string_view src) : node(node), source(src) {}

	operator TSNode& () { return node; }
	operator const TSNode& () const { return node; }

	operator bool() const { return !ts_node_is_null(node); }
	bool operator!() const { return ts_node_is_null(node); }

	bool HasErrors() const { return ts_node_has_error(node); }
	bool IsMissing() const { return ts_node_is_missing(node); }
	bool IsNamed() const { return ts_node_is_named(node); }

	TreeNode GetChild(size_t index) const { return { ts_node_child(node, index), source }; }
	TreeNode GetNamedChild(size_t index) const { return { ts_node_named_child(node, index), source }; }
	
	TreeNode FindChild(std::string_view name) const;

	std::vector<TreeNode> GatherChildren() const;
	std::vector<TreeNode> GatherChildren(std::string_view name) const;
	std::vector<TreeNode> GatherChildren(bool(*pred)(const TreeNode&)) const;

	std::vector<TreeNode> GatherNamedChildren() const;
	
	//template <typename...Ts> requires (std::convertible_to<Ts, std::string_view> && ...)
	//std::array<TreeNode, sizeof...(Ts)> GatherOrderedChildren(Ts&&...ts)
	//{
	//	static constexpr auto findOne = []<size_t I>(std::string_view name)

	//	std::array<TreeNode, sizeof...(Ts)> children;
	//	for (uint32_t i = 0; i < ts_node_child_count(node); ++i)
	//	{
	//		if (auto child = TreeNode(ts_node_child(node, i), source);
	//			child.GetType() == name)
	//		{
	//			children.push_back(child);
	//		}
	//	}

	//	return children;
	//}

	//template <FixedString...strs>
	//std::array<TreeNode, sizeof...(strs)> GatherOrderedChildren()
	//{
	//	std::array<TreeNode, sizeof...(strs)> children;

	//	static constexpr auto findOne = []<size_t I>(auto&& name)

	//	for (uint32_t i = 0; i < ts_node_child_count(node); ++i)
	//	{
	//		if (auto child = TreeNode(ts_node_child(node, i), source);
	//			child.GetType() == name)
	//		{
	//			children.push_back(child);
	//		}
	//	}

	//	return children;
	//}

	size_t GetChildCount() const { return ts_node_child_count(node); }
	size_t GetNamedChildCount() const { return ts_node_named_child_count(node); }

	std::string_view GetText() const;
	std::string_view GetType() const;

	size_t GetStartPos() const { return ts_node_start_byte(node); }
	size_t GetEndPos() const { return ts_node_end_byte(node); }
	TSPoint GetStartPoint() const { return ts_node_start_point(node); }
	TSPoint GetEndPoint() const { return ts_node_end_point(node); }
	SourceLocation GetSourceLocation() const { return { GetStartPos(), GetEndPos() }; }

	TSNode node;
	std::string_view source;
};


} // ui

#endif