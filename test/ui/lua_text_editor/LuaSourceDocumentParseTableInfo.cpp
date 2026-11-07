#include "LuaSourceDocumentParseTableInfo.h"

#if IMGUI_ENABLED
#include "TreeCursor.h"
#include "LuaSourceParserCommon.h"
#include "LuaSourceDocumentSymbol.h"
#include <ctre/ctre.hpp>

namespace ui {

bool ParseTableInfo(TreeCursor& cursor, LuaSourceDocumentSymbolParseContext& ctx)
{
    if (!cursor.GoToFirstChild())
    {
        ctx.AddError(cursor.CurrentNode(), "Failed to go to first child of root cursor");

        return false;
    }

    do
    {
        if (cursor.CurrentNode().GetType() != NodeName::kFunctionCall)
        {
            continue;
        }
        if (!cursor.GoToFirstChild())
        {
            continue;
        }

        do
        {
            if (!IsMetatableNode(cursor.CurrentNode()))
            {
                continue;
            }

            while (cursor.GoToNextSibling())
            {
                auto node = cursor.CurrentNode();

                if (node.GetType() != NodeName::kArguments)
                {
                    continue;
                }

                if (node.GetNamedChildCount() != 2)
                {
                    ctx.AddError(node, "Expected 2 named children in metatable args node");

                    return false;
                }

                if (!cursor.GoToFirstNamedChild())
                {
                    ctx.AddError(node, "Failed to go to "
                        "first named child of metatable args node");

                    return false;
                }

                node = cursor.CurrentNode();
                if (node.GetType() != NodeName::kIdentifier)
                {
                    ctx.AddError(node, "Expected first named child of "
                        "metatable args node to be an identifier");

                    return false;
                }

                ctx.tableInfo.functionTableName = node.GetText();

                if (!cursor.GoToNextNamedSibling())
                {
                    ctx.AddError(node, "Failed to go to "
                        "second named child of metatable args node");

                    return false;
                }

                node = cursor.CurrentNode();
                if (node.GetType() != NodeName::kIdentifier)
                {
                    ctx.AddError(node, "Expected second named child of "
                        "metatable args node to be an identifier");

                    return false;
                }

                ctx.tableInfo.metaTableName = node.GetText();

                return true;
            }

        } while (cursor.GoToNextSibling());

        cursor.GoToParent();

    } while (cursor.GoToNextSibling());

    ctx.AddError(cursor.CurrentNode(), 
        "Failed to parse metatable function call and arguments");

    return false;
}

} // ui

#endif