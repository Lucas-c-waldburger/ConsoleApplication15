#include "TreeCursor.h"

#if IMGUI_ENABLED
#include <cassert>
#include "LuaSourceParser.h"

namespace ui {

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

bool TreeCursor::GoToChildAtByte(uint32_t byte)
{
    while (ts_tree_cursor_goto_first_child_for_byte(&cursor, byte) != -1) 
    {
        ++homeDepth;
    }

    auto node = CurrentNode();

    return !ts_node_is_null(node) && ts_node_start_byte(node) == byte;
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

} // ui

#endif