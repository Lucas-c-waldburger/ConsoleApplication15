#pragma once
#include "TreeNode.h"

#if IMGUI_ENABLED
#include <vector>

namespace ui {

struct TreeCursor
{
	TreeCursor() : cursor{}, source{} {}
	TreeCursor(TSNode node, std::string_view src) : cursor(ts_tree_cursor_new(node)), source(src) {}
	~TreeCursor() { ts_tree_cursor_delete(&cursor); }
	TreeCursor(const TreeCursor& other) = delete;
	TreeCursor& operator=(const TreeCursor& other) = delete;
	TreeCursor(TreeCursor&& other) noexcept : cursor(other.cursor), source(other.source), 
		homeDepth(other.homeDepth)
	{
		other.cursor = {};
	}
	TreeCursor& operator=(TreeCursor&& other) noexcept
	{
		if (this == &other) { return *this; }

		cursor = other.cursor;
		source = other.source;
		homeDepth = other.homeDepth;
		other.cursor = {};
		return *this;
	}

	operator bool() const { return !source.empty(); }
	bool operator!() const { return source.empty(); }

	TreeNode CurrentNode() const { return TreeNode(ts_tree_cursor_current_node(&cursor), source); }

	bool GoToFirstChild();
	bool GoToNextSibling() { return ts_tree_cursor_goto_next_sibling(&cursor); }
	bool GoToPrevSibling() { return ts_tree_cursor_goto_previous_sibling(&cursor); }
	bool GoToParent();
	bool GoToParent(size_t lvls);
	void Reset();
	bool GoToFirstNamedChild();
	bool GoToNextNamedSibling();

	void SetHomeDepth() { homeDepth = 0; }
	bool GoToHomeDepth();

	TreeCursor Copy() const;
	TreeCursor Branch() const;

	/*std::vector<TreeNode> GatherChildren(std::string_view name);
	std::vector<TreeNode> GatherChildren(std::string_view name, 
										 std::vector<SourceDocumentError>& errors);
	std::vector<TreeNode> GatherChildren(bool(*pred)(const TreeNode&));
	std::vector<TreeNode> GatherChildren(bool(*pred)(const TreeNode&),
											 std::vector<SourceDocumentError>& errors);

	template <typename Pred>
	std::vector<TreeNode> GatherChildren(Pred&& pred)
	{
		std::vector<TreeNode> matches;

		if (!GoToFirstChild())
		{
			return matches;
		}

		do
		{
			auto node = CurrentNode();
			if (pred(node))
			{
				matches.push_back(node);
			}
		} while (GoToNextSibling());

		return matches;
	}*/

	/*template <auto...preds>
	std::vector<TreeNode> Traverse() requires (sizeof...(preds) > 0)
	{
		static constexpr auto traverseOne = []<auto pred, size_t remaining>(
			TreeCursor& c, std::vector<TreeNode>& matches) -> bool
		{
			if (!c.GoToFirstChild())
			{
				return false;
			}

			do
			{
				auto node = c.CurrentNode();

				if (pred(node))
				{
					matches.push_back(node);
				}
			} while (c.GoToNextSibling());
		};

		return (traverseOne.template operator()<preds>(*this) || ...);
	}*/

	TSTreeCursor cursor;
	std::string_view source;
	int homeDepth = 0;
};


} // ui

#endif