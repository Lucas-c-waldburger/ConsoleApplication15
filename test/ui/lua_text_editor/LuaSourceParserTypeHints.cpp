#include "LuaSourceParserTypeHints.h"

#if IMGUI_ENABLED
#include "TreeCursor.h"
#include "LuaSourceDocumentSymbol.h"
#include <vector>
#include <cassert>

namespace ui {

// param structure (':' associated with lhs)
// - parameters
// --- (
// --- [ERROR]
// ----- identifier   *lhs*
// ----- ':'
// --- identifier     *rhs*
// --- )

// param structure (':' associated with rhs)
// - parameters
// --- (
// --- identifier    *lhs*
// --- [ERROR]
// ----- ':'
// ----- identifier  *rhs*
// --- )

std::optional<ParsedVariableInfo> ParseSingleParameter(TreeCursor& cursor)
{
	ParsedVariableInfo info{
		.scopeType = VariableScopeType::Parameter
	};
	
	auto node = cursor.CurrentNode();

	if (node.HasErrors()) // ':' grouped with lhs
	{
		if (node.GetChildCount() != 2)
		{
			cursor.GoToParent();
			return std::nullopt;
		}
		if (!cursor.GoToFirstChild())
		{
			cursor.GoToParent();
			return std::nullopt;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kIdentifier)
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}

		info.name = node.GetText();
		if (info.name.empty())
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}

		if (!cursor.GoToNextSibling())
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}
		
		node = cursor.CurrentNode();
		if (node.GetType() != ":")
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}

		info.errorSpan.start = node.GetSourceLocation().start;
	}
	else if (node.GetType() != NodeName::kIdentifier) // no type hint
	{
		info.name = node.GetText();

		cursor.GoToParent();

		return (!info.name.empty()) 
			? std::optional<ParsedVariableInfo>{std::move(info)} 
			: std::optional<ParsedVariableInfo>{};
	}

	// find type from hint
	if (!cursor.GoToNextSibling())
	{
		cursor.GoToParent();
		return std::nullopt;
	}

	node = cursor.CurrentNode();
	if (node.GetType() != NodeName::kIdentifier)
	{
		cursor.GoToParent();
		return std::nullopt;
	}

	info.typeText = node.GetText();
	info.errorSpan.end = node.GetSourceLocation().end;

	cursor.GoToParent();

	return (!info.typeText.empty()) 
		? std::optional<ParsedVariableInfo>{std::move(info)} 
		: std::optional<ParsedVariableInfo>{};
}
 
std::vector<ParsedVariableInfo> ParseParameters(TreeCursor& cursor)
{
	assert(cursor.CurrentNode().GetType() == NodeName::kParameters);

	if (!cursor.GoToFirstChild())
	{ 
		return {};
	}
	if (cursor.CurrentNode().GetType() != "(")
	{
		cursor.GoToParent();
		return {};
	}
	if (!cursor.GoToNextSibling())
	{
		cursor.GoToParent();
		return {};
	}

	std::vector<ParsedVariableInfo> params;
	do
	{
		if (cursor.CurrentNode().GetType() == ")")
		{
			continue;
		}

		if (auto info = ParseSingleParameter(cursor))
		{
			params.emplace_back(std::move(*info));
		}

	} while (cursor.GoToNextSibling());

	cursor.GoToParent();

	return params;
}

// no type hint: 
// - variable_declaration 
// --- (local) 
// --- assignment_statement 
// ----- variable_list

// type hint: 
// - variable_declaration
// --- (local) 
// --- variable_list
// - [ERROR]
std::optional<ParsedVariableInfo> ParseVariableDeclaration(TreeCursor& cursor)
{
	ParsedVariableInfo info{};

	assert(cursor.CurrentNode().GetType() == NodeName::kVariableDeclaration);

	if (!cursor.GoToFirstChild())
	{
		return std::nullopt;
	}

	auto node = cursor.CurrentNode();

	if (node.GetType() == kLocalName)
	{
		info.scopeType = VariableScopeType::Local;

		if (!cursor.GoToNextSibling())
		{
			cursor.GoToParent();
			return std::nullopt;
		}

		node = cursor.CurrentNode();
	}

	if (node.GetType() == NodeName::kVariableList) // maybe type hint
	{
		if (node.GetChildCount() != 1)
		{
			cursor.GoToParent();
			return std::nullopt;
		}

		if (!cursor.GoToFirstChild())
		{
			cursor.GoToParent();
			return std::nullopt;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kIdentifier)
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}

		info.name = node.GetText();
		if (info.name.empty())
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}

		cursor.GoToParent();

		if (!cursor.GoToNextSibling())
		{
			cursor.GoToParent();
			return std::nullopt;
		}

		node = cursor.CurrentNode();
		if (!node.HasErrors() || node.GetChildCount() != 1)
		{
			cursor.GoToParent();
			return std::nullopt;
		}

		if (!cursor.GoToFirstChild())
		{
			cursor.GoToParent();
			return std::nullopt;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != ":")
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}

		info.errorSpan.start = node.GetSourceLocation().start;

		cursor.GoToParent();

		if (!cursor.GoToNextSibling())
		{
			cursor.GoToParent();
			return std::nullopt;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kAssignmentStatement ||
			node.GetNamedChildCount() != 2)
		{
			cursor.GoToParent();
			return std::nullopt;
		}

		if (!cursor.GoToFirstChild())
		{
			cursor.GoToParent();
			return std::nullopt;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kVariableList ||
			node.GetChildCount() != 1)
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}

		if (!cursor.GoToFirstChild())
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kIdentifier)
		{
			cursor.GoToParent(3);
			return std::nullopt;
		}

		info.typeText = node.GetText();
		if (info.typeText.empty())
		{
			cursor.GoToParent(3);
			return std::nullopt;
		}

		info.errorSpan.end = node.GetSourceLocation().end;

		cursor.GoToParent();

		if (!cursor.GoToNextNamedSibling()) // to expression_list
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}

		if (cursor.CurrentNode().GetType() == NodeName::kExpressionList)
		{
			info.rhsCursor = cursor.Branch();
		}

		cursor.GoToParent();

		cursor.GoToNextNamedSibling();
	}
	else if (node.GetType() == NodeName::kAssignmentStatement)
	{
		if (node.GetNamedChildCount() != 2)
		{
			cursor.GoToParent();
			return std::nullopt;
		}

		if (!cursor.GoToFirstChild())
		{
			cursor.GoToParent();
			return std::nullopt;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kVariableList ||
			node.GetChildCount() != 1)
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}

		if (!cursor.GoToFirstChild())
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kIdentifier)
		{
			cursor.GoToParent(3);
			return std::nullopt;
		}

		info.name = node.GetText();
		if (info.name.empty())
		{
			cursor.GoToParent(3);
			return std::nullopt;
		}

		cursor.GoToParent(1);

		if (!cursor.GoToNextNamedSibling()) // to expression_list
		{
			cursor.GoToParent(2);
			return std::nullopt;
		}

		if (cursor.CurrentNode().GetType() == NodeName::kExpressionList)
		{
			info.rhsCursor = cursor.Branch();
		}

		cursor.GoToParent(2);

		cursor.GoToNextNamedSibling();
	}
}

} // ui

#endif