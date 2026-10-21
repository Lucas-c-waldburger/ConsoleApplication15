#include "TreeCursor.h"

#if IMGUI_ENABLED
#include <cassert>
#include "LuaSourceParser.h"

namespace ui {

//TreeCursor::TreeCursor(const TreeCursor& other) : 
//    cursor(ts_tree_cursor_new(other.CurrentNode())), 
//    source(other.source), homeDepth(0)
//{}
//
//TreeCursor& TreeCursor::operator=(const TreeCursor& other)
//{
//    if (this == &other) { return *this; }
//
//    cursor = ts_tree_cursor_new(other.CurrentNode());
//    source = other.source;
//    homeDepth = 0;
//
//    return *this;
//}

bool TreeCursor::GoToFirstChild()
{
    if (!ts_tree_cursor_goto_first_child(&cursor)) 
    {
        return false;
    }

    ++homeDepth;

    return true;
}

bool TreeCursor::GoToParent()
{
    if (!ts_tree_cursor_goto_parent(&cursor))
    {
        return false;
    }

    --homeDepth;

    return true;
}

bool TreeCursor::GoToParent(size_t lvls)
{
    for (size_t i = 0; i < lvls; ++i)
    {
        if (!GoToParent())
        {
            return false;
        }
    }

    return true;
}

bool TreeCursor::GoToFirstNamedChild()
{
    if (!GoToFirstChild())
    {
        return false;
    }
    do
    {
        if (CurrentNode().IsNamed())
        {
            return true;
        }
    } while (GoToNextSibling());

    GoToParent();

    return false;
}

bool TreeCursor::GoToNextNamedSibling()
{
    if (!GoToNextSibling())
    {
        return false;
    }
    do
    {
        if (CurrentNode().IsNamed())
        {
            return true;
        }
    } while (GoToNextSibling());

    return false;
}

bool TreeCursor::GoToHomeDepth()
{
    while (homeDepth != 0)
    {
       if (!(homeDepth > 0 ? GoToParent() : GoToFirstChild()))
       {
           return false;
	   }
    }

	assert(homeDepth == 0);

	return true;
}

TreeCursor TreeCursor::Copy() const
{
    TreeCursor copy{};
    copy.cursor = ts_tree_cursor_copy(&cursor);
    copy.source = source;
    copy.homeDepth = homeDepth;

    return copy;
}

TreeCursor TreeCursor::Branch() const
{
    return TreeCursor(CurrentNode(), source);
}

void TreeCursor::Reset()
{
    while (GoToParent()) {}

    homeDepth = 0;
}

//std::vector<TreeNode> TreeCursor::GatherChildren(std::string_view name)
//{
//    std::vector<TreeNode> children;
//    if (!GoToFirstNamedChild())
//    {
//        return children;
//    }
//    do
//    {
//        if (auto node = CurrentNode(); node.GetType() == name)
//        {
//            children.push_back(node);
//        }
//
//    } while (GoToNextNamedSibling());
//
//    return children;
//}
//
//std::vector<TreeNode> TreeCursor::GatherChildren(std::string_view name, 
//                                                 std::vector<SourceDocumentError>& errors)
//{
//    std::vector<TreeNode> children;
//
//    if (!GoToFirstNamedChild())
//    {
//		auto node = CurrentNode();
//
//        errors.emplace_back(
//            std::format("Failed to go to first child of node '{}'", node.GetType()),
//            node.GetSourceLocation());
//
//		return children;
//    }
//    do
//    {
//        if (auto node = CurrentNode(); node.GetType() == name)
//        {
//            children.emplace_back(node);
//        }
//
//    } while (GoToNextNamedSibling());
//
//    return children;
//}
//
//std::vector<TreeNode> TreeCursor::GatherChildren(bool(*pred)(const TreeNode&))
//{
//    std::vector<TreeNode> children;
//    if (!GoToFirstNamedChild())
//    {
//        return children;
//    }
//    do
//    {
//        if (auto node = CurrentNode(); pred(node))
//        {
//            children.push_back(node);
//        }
//
//    } while (GoToNextNamedSibling());
//
//    return children;
//}
//
//std::vector<TreeNode> TreeCursor::GatherChildren(bool(*pred)(const TreeNode&), 
//                                                 std::vector<SourceDocumentError>& errors)
//{
//    std::vector<TreeNode> children;
//    if (!GoToFirstNamedChild())
//    {
//        if (!GoToFirstNamedChild())
//        {
//            auto node = CurrentNode();
//
//            errors.emplace_back(
//                std::format("Failed to go to first child of node '{}'", node.GetType()),
//                node.GetSourceLocation());
//
//            return children;
//        }
//    }
//    do
//    {
//        if (auto node = CurrentNode(); pred(node))
//        {
//            children.push_back(node);
//        }
//
//    } while (GoToNextNamedSibling());
//
//    return children;
//}

} // ui

#endif