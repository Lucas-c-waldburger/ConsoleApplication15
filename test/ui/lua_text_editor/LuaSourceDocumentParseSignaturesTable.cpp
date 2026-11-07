#include "LuaSourceDocumentParseSignaturesTable.h"

#if IMGUI_ENABLED
#include "TreeCursor.h"
#include "LuaSourceParserCommon.h"
#include "LuaSourceDocumentSymbol.h"
#include <ctre/ctre.hpp>

namespace ui {

namespace {

constexpr std::string_view 
GetSignaturesParameterRawTypeName(std::string_view typeNameWithCvRefQualifiers)
{
	static constexpr auto regex = ctll::fixed_string{
		R"(^\s*(?:const\s+)?(?<type>[A-Za-z_][A-Za-z0-9_]*)(?:\s+const)?\s*[&*]?\s*$)"
	};

	auto match = ctre::match<regex>(typeNameWithCvRefQualifiers);
	if (!match)
	{
		return {};
	}

	return match.get<"type">().to_view();
}

std::string_view GetSignaturesFieldNodeStringContent(TreeCursor& cursor)
{
	assert(cursor.CurrentNode().GetType() == NodeName::kField);

	if (!cursor.GoToFirstNamedChild() ||
		cursor.CurrentNode().GetType() != NodeName::kString)
	{
		return {};
	}
	if (!cursor.GoToFirstNamedChild() ||
		cursor.CurrentNode().GetType() != NodeName::kStringContent)
	{
		return {};
	}

	return cursor.CurrentNode().GetText();
}

bool ParseSignaturesTableImpl(TreeCursor& cursor, LuaSourceDocumentSymbolParseContext& ctx)
{
	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kTableConstructor);

	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(node,
			"Failed to go to first child of signatures table constructor node");

		return false;
	} 

	cursor.SetHomeDepth();

	do
	{
		if (cursor.CurrentNode().GetType() != NodeName::kField)
		{
			continue;
		}
		if (!cursor.GoToFirstNamedChild())
		{
			continue;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kIdentifier)
		{
			ctx.AddError(node,
				"Expected first child of signature field node to be identifier");

			cursor.GoToHomeDepth();
			continue;
		}

		auto fnIdent = node.GetText();
		if (fnIdent.empty())
		{
			ctx.AddError(node, "Signature field identifier text was empty");

			cursor.GoToHomeDepth();
			continue;
		}

		if (!GoToSiblingWithNodeName(cursor, NodeName::kTableConstructor))
		{
			ctx.AddError(cursor.CurrentNode(),
				"Expected second node in signatures table "
				"field to be table_constructor (parameter types)");

			cursor.GoToHomeDepth();
			continue;
		}

		ctx.signaturesTable.functionNames.emplace_back(fnIdent);
		auto& paramNames = ctx.signaturesTable.parameterNames.emplace_back();
		auto& paramTypes = ctx.signaturesTable.parameterTypes.emplace_back();

		auto paramCursor = cursor.Branch();
		cursor.GoToHomeDepth();

		if (!paramCursor.GoToFirstNamedChild())
		{
			// assume no parameters for function
			continue;
		}

		paramCursor.SetHomeDepth();

		do // gather parameter types
		{
			if (paramCursor.CurrentNode().GetType() != NodeName::kField)
			{
				continue;
			}

			auto paramType = GetSignaturesFieldNodeStringContent(paramCursor);
			auto rawParamType = GetSignaturesParameterRawTypeName(paramType);

			uint32_t typeId = kInvalidLuaTypeId;
			if (!rawParamType.empty())
			{
				if (auto dataIdx = ctx.userTypeInfo.GetDataIndex(rawParamType);
					dataIdx.IsValid())
				{
					typeId = ctx.userTypeInfo.GetUserTypeId(dataIdx);
				}
			}
 			paramTypes.emplace_back(typeId);
 			paramNames.emplace_back();

			paramCursor.GoToHomeDepth();

		} while (paramCursor.GoToNextNamedSibling());

	} while (cursor.GoToNextNamedSibling());

	return true;
}

} // unnamed

bool ParseSignaturesTable(TreeCursor& cursor, LuaSourceDocumentSymbolParseContext& ctx)
{
	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(cursor.CurrentNode(),
			"Failed to go to first child of root cursor");

		return false;
	}

	cursor.SetHomeDepth();

	do // loop through all root children
	{
		auto node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kVariableDeclaration)
		{
			continue;
		}

		if (!cursor.GoToFirstNamedChild())
		{
			continue;
		}

		// Find 'assignment_statement' child of this variable_declaration
		if (!GoToSiblingWithNodeName(cursor, NodeName::kAssignmentStatement))
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Now cursor is on assignment_statement. Check if its variable_list contains our metatable name.
		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Search for variable_list
		if (!GoToSiblingWithNodeName(cursor, NodeName::kVariableList))
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Now cursor is on variable_list. Search for identifier matching metaTableName.
		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		assert(!ctx.tableInfo.metaTableName.empty());

		// find identifier with text matching metaTableName
		if (!GoToSiblingWithNodeNameAndText(cursor, NodeName::kIdentifier,
										    ctx.tableInfo.metaTableName))
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Found the metatable identifier. 
		// go back to assignment_statement and find expression_list.
		cursor.GoToParent(2); // identifier -> variable_list -> assignment_statement

		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Find expression_list sibling in assignment_statement
		if (!GoToSiblingWithNodeName(cursor, NodeName::kExpressionList))
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Cursor is now on expression_list. Search for table_constructor.
		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		if (!GoToSiblingWithNodeName(cursor, NodeName::kTableConstructor))
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Cursor is on table_constructor.
		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		do
		{
			if (cursor.CurrentNode().GetType() != NodeName::kField)
			{
				continue;
			}

			if (!cursor.GoToFirstNamedChild())
			{
				continue;
			}

			if (!IsSignaturesNode(cursor.CurrentNode()))
			{
				cursor.GoToParent(); // back to field 
				continue;
			}

			// next sib should be table_constructor
			if (!GoToSiblingWithNodeName(cursor, NodeName::kTableConstructor))
			{
				cursor.GoToParent(); // back to field
				continue;
			}

			auto signaturesCursor = cursor.Branch();
			cursor.GoToParent();

			// get table function names + param types
			return ParseSignaturesTableImpl(signaturesCursor, ctx);

		} while (cursor.GoToNextNamedSibling());

		// If we reach here, we didn't find __signatures in this variable_declaration.
		// Go back to root list and continue with the next one.
		cursor.GoToHomeDepth();

	} while (cursor.GoToNextSibling());

	ctx.AddError(cursor.CurrentNode(), "Could not parse signatures table");

	return false;
}

} // ui

#endif