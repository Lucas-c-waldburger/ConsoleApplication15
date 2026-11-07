#include "LuaSourceDocumentSymbolParser.h"

#if IMGUI_ENABLED
#include "TreeCursor.h"

namespace ui::lua {

namespace {

using Sym = LuaSourceDocumentSymbol;

struct TypedIdentifierInfo
{
	std::string_view nameText;
	std::string_view typeText;
	Sym::Kind kind = Sym::Kind::Invalid;
	SourceLocation typeSpan;
	TreeCursor rhsCursor;

	bool HasTypeHint() const noexcept
	{
		return !typeText.empty() && typeSpan.start < typeSpan.end;
	}

	bool HasRhs() const noexcept
	{
		return rhsCursor;
	}
};

bool NodeStartsNewScope(const TreeNode& node)
{
	return node.GetType() == NodeName::kBlock ||
		   node.GetType() == NodeName::kFunctionDeclaration;
}

bool NodeEndsCurrentScope(const TreeNode& node,
						  const LuaSourceDocumentScopeTree& scopeTree)
{
	assert(scopeTree.CurrentValid());

	return !scopeTree.AtRoot() && 
		   node.GetSourceLocation().start >=
		   scopeTree.GetCurrentScopeLocation().end;
}

constexpr bool MemberKindMatchesIndexExprType(LuaUsertypeMemberKind kind,
											  IndexExpressionType type) noexcept
{
	return (type == IndexExpressionType::Dot && IsMemberKindField(kind)) ||
		   (type == IndexExpressionType::Method && IsMemberKindFunction(kind));
}

// Go through previously added symbols and find the one with same name & scope
size_t GetSymIndexForIdentifier(std::string_view ident, SourceLocation newSymLoc,
								LuaSourceDocumentParseContext& ctx)
{
	for (size_t i = ctx.symbols.Size(); i-- > 0;)
	{
		const auto [symName, symScope] = ctx.symbols.GetView<&Sym::name, &Sym::scope>(i);

		if (symName != ident)
		{
			continue;
		}

		if (newSymLoc.Inside(symScope))
		{
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

Sym::Kind GetSymKindAndAdvance(TreeCursor& cursor, 
							   LuaSourceDocumentScopeTree& scopeTree)
{
	auto node = cursor.CurrentNode();
	if (node.GetType() == kLocalName)
	{
		if (cursor.GoToNextNamedSibling())
		{
			return Sym::Kind::Local;
		}
		
		return Sym::Kind::Invalid;
	}
	else if (node.GetType() == kGlobalName)
	{
		if (!cursor.GoToNextNamedSibling())
		{
			return Sym::Kind::Invalid;
		}
	}

	return (scopeTree.InsideFunctionScope())
		? Sym::Kind::ScopedGlobal
		: Sym::Kind::UnscopedGlobal;
}

std::optional<TypedIdentifierInfo>
ParseVariableDeclaration(TreeCursor& cursor, LuaSourceDocumentParseContext& ctx)
{
	std::optional<TypedIdentifierInfo> info{std::in_place};

	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kVariableDeclaration);

	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(node, "Failed to go to first child of variable_declaration node");
		return std::nullopt;
	}

	size_t depth = 1;

	info->kind = GetSymKindAndAdvance(cursor, ctx.scopeTree);
	if (info->kind == Sym::Kind::Invalid)
	{
		ctx.AddError(cursor, "Failed to go to node after local/global");
		cursor.GoToParent(depth);

		return std::nullopt;
	}

	bool isAssignStmt = false;

	node = cursor.CurrentNode();
	if (node.GetType() == NodeName::kVariableList)
	{
		if (node.GetChildCount() != 1)
		{
			ctx.AddError(cursor, "variable_list node did not have one child");

			cursor.GoToParent(depth);
			return std::nullopt;
		}

		if (!cursor.GoToFirstChild())
		{
			ctx.AddError(cursor, "failed to go to first child of variable_list node");

			cursor.GoToParent();
			return std::nullopt;
		}
		++depth;

		if (cursor.CurrentNode().GetType() != NodeName::kTypedIdentifier)
		{
			ctx.AddError(cursor, 
				"first child of variable_list node was not typed_identifier");

			cursor.GoToParent(depth);
			return std::nullopt;
		}
	}
	else if (node.GetType() == NodeName::kAssignmentStatement)
	{
		isAssignStmt = true;

		if (node.GetNamedChildCount() != 2)
		{
			ctx.AddError(cursor, 
				"assignment_statement node did not have two named children");

			cursor.GoToParent(depth);
			return std::nullopt;
		}
		if (!cursor.GoToFirstChild())
		{
			ctx.AddError(cursor, 
				"failed to go to first child of assignment_statement node");
			cursor.GoToParent(depth);

			return std::nullopt;
		}
		++depth;

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kVariableList)
		{
			ctx.AddError(node,
				"first child of assignment_statement node was not variable_list");

			cursor.GoToParent(depth);
			return std::nullopt;
		}

		if (node.GetChildCount() != 1)
		{
			ctx.AddError(node,
				"variable_list node did not have one child");

			cursor.GoToParent(depth);
			return std::nullopt;
		}

		if (!cursor.GoToFirstChild())
		{
			ctx.AddError(cursor, 
				"failed to go to first child of variable_list node");

			cursor.GoToParent(depth);
			return std::nullopt;
		}
		++depth;

		if (cursor.CurrentNode().GetType() != NodeName::kTypedIdentifier)
		{
			ctx.AddError(cursor, 
				"first child of variable_list node was not typed_identifier");

			cursor.GoToParent(depth);
			return std::nullopt;
		}
	}
	else
	{
		ctx.AddError(node, 
			"variable_declaration node did not have variable_list "
			"or assignment_statement child");

		cursor.GoToParent(depth);
		return std::nullopt;
	}

	assert(cursor.CurrentNode().GetType() == NodeName::kTypedIdentifier);

	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(cursor, "failed to go to first child of typed_identifier node");

		cursor.GoToParent(depth);
		return std::nullopt;
	}
	++depth;

	node = cursor.CurrentNode();
	if (node.GetType() != NodeName::kIdentifier)
	{
		ctx.AddError(node, "first child of typed_identifier node was not identifier");

		cursor.GoToParent(depth);
		return std::nullopt;
	}

	info->nameText = node.GetText();
	if (info->nameText.empty())
	{
		ctx.AddError(node, "identifier text was empty");

		cursor.GoToParent(depth);
		return std::nullopt;
	}

	if (cursor.GoToNextSibling()) // check for type hint
	{
		node = cursor.CurrentNode();
		if (node.GetType() != ":")
		{
			ctx.AddError(node, 
				std::format("Expected ':' node after identifier, got {}", node.GetType()));

			cursor.GoToParent(depth);
			return std::nullopt;
		}

		info->typeSpan.start = node.GetSourceLocation().start;

		if (!cursor.GoToNextSibling())
		{
			ctx.AddError(cursor, "failed to go to next sibling after ':' node");

			cursor.GoToParent(depth);
			return std::nullopt;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kIdentifier)
		{
			ctx.AddError(node, 
				"second named child of typed_identifier node was not identifier");

			cursor.GoToParent(depth);
			return std::nullopt;
		}

		info->typeText = node.GetText();
		if (info->typeText.empty())
		{
			ctx.AddError(node, "identifier text was empty");
	
			cursor.GoToParent(depth);
			return std::nullopt;
		}

		info->typeSpan.end = node.GetSourceLocation().end;
	}

	if (!isAssignStmt)
	{
		cursor.GoToParent(depth);

		return info;
	}

	cursor.GoToParent(2); // back to variable_list
	----depth;

	if (!cursor.GoToNextNamedSibling()) // to expression_list
	{
		ctx.AddError(cursor, "failed to go to expression_list node after variable_list");

		cursor.GoToParent(depth);
		return std::nullopt;
	}

	node = cursor.CurrentNode();
	if (node.GetType() != NodeName::kExpressionList)
	{
		ctx.AddError(node, 
			std::format("Expected expression_list node, got {}", node.GetType()));

		cursor.GoToParent(depth);
		return std::nullopt;
	}

	if (node.GetNamedChildCount() != 1)
	{
		ctx.AddError(node, "expression_list node did not have one named child");

		cursor.GoToParent(depth);
		return std::nullopt;
	}

	if (!cursor.GoToFirstNamedChild())
	{
		ctx.AddError(cursor, "failed to go to first named child of expression_list node");

		cursor.GoToParent(depth);
		return std::nullopt;
	}
	++depth;

	info->rhsCursor = cursor.Branch();

	cursor.GoToParent(depth);

	return info;
}

std::optional<TypedIdentifierInfo>
ParseSingleParameter(TreeCursor& cursor, LuaSourceDocumentParseContext& ctx)
{
	assert(cursor.CurrentNode().GetType() == NodeName::kTypedIdentifier);

	std::optional<TypedIdentifierInfo> info{std::in_place};
	info->kind = Sym::Kind::Parameter;

	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(cursor, "failed to go to first child of typed_identifier node");

		return std::nullopt;
	}

	auto node = cursor.CurrentNode();
	if (node.GetType() != NodeName::kIdentifier)
	{
		ctx.AddError(node, "first child of typed_identifier node was not identifier");

		cursor.GoToParent();
		return std::nullopt;
	}

	info->nameText = node.GetText();
	if (info->nameText.empty())
	{
		ctx.AddError(node, "identifier text was empty");

		cursor.GoToParent();
		return std::nullopt;
	}

	if (!cursor.GoToNextSibling()) // check for type hint
	{
		cursor.GoToParent();
		return info;
	}

	node = cursor.CurrentNode();
	if (node.GetType() != ":")
	{
		ctx.AddError(node, 
			std::format("Expected ':' node after identifier, got {}", node.GetType()));

		cursor.GoToParent();
		return std::nullopt;
	}

	info->typeSpan.start = node.GetSourceLocation().start;

	if (!cursor.GoToNextSibling())
	{
		ctx.AddError(cursor, "failed to go to next sibling after ':' node");

		cursor.GoToParent();
		return std::nullopt;
	}

	node = cursor.CurrentNode();
	if (node.GetType() != NodeName::kIdentifier)
	{
		ctx.AddError(node, 
			"second named child of typed_identifier node was not identifier");

		cursor.GoToParent();
		return std::nullopt;
	}

	info->typeText = node.GetText();
	if (info->typeText.empty())
	{
		ctx.AddError(node, "identifier text was empty");

		cursor.GoToParent();
		return std::nullopt;
	}

	info->typeSpan.end = node.GetSourceLocation().end;

	cursor.GoToParent();

	return info;
}

std::vector<TypedIdentifierInfo>
ParseParameters(TreeCursor& cursor, LuaSourceDocumentParseContext& ctx)
{
	assert(cursor.CurrentNode().GetType() == NodeName::kParameters);

	if (!cursor.GoToFirstChild())
	{
		return {};
	}

	if (cursor.CurrentNode().GetType() != "(")
	{
		ctx.AddError(cursor, 
			std::format("Expected '(' node, got {}", cursor.CurrentNode().GetType()));

		cursor.GoToParent();
		return {};
	}

	std::vector<TypedIdentifierInfo> params;
	size_t posCounter = 0;

	while (cursor.GoToNextSibling())
	{
		auto node = cursor.CurrentNode();
		if (node.GetType() == ")")
		{
			break;
		}

		if (node.GetType() != NodeName::kTypedIdentifier)
		{
			ctx.AddError(node, 
				std::format("Expected typed_identifier node, got {}", node.GetType()));
			continue;
		}
		if (auto paramInfo = ParseSingleParameter(cursor, ctx))
		{
			params.emplace_back(std::move(*paramInfo));
		}
		else
		{
			ctx.AddError(cursor, 
				std::format("Failed to parse parameter at position {}", posCounter));
		}

		++posCounter;
	}

	return params;
}

enum OutFlag
{
	FunctionCall = 1 << 0,
};

struct ResolvedListInfo
{
	enum Flag
	{
		FunctionCall = 1 << 0,
	};

	std::vector<std::string_view> identifiers;
	std::vector<IndexExpressionType> indexExprTypes;
	uint8_t flags = 0;

	bool MaybeEnum() const noexcept
	{
		return identifiers.size() == 2 &&
			   indexExprTypes.size() == 1 &&
			   indexExprTypes[0] == IndexExpressionType::Dot;
	}

	bool IsSingleIdentifier() const noexcept
	{
		if (identifiers.size() == 1)
		{
			assert(indexExprTypes.empty());
			return true;
		}
		return false;
	}
};

bool GetResolvedListInfoImpl(TreeCursor& cursor, ResolvedListInfo& info)
{
	auto node = cursor.CurrentNode();
	if (node.GetType() == NodeName::kIdentifier)
	{
		info.identifiers.emplace_back(node.GetText());

		return true;
	}
	else if (IsIndexExpressionNode(node))
	{
		info.indexExprTypes.emplace_back((node.GetType() == NodeName::kDotIndexExpression
			? IndexExpressionType::Dot
			: IndexExpressionType::Method));

		if (!cursor.GoToFirstChild())
		{
			return false;
		}

		if (!GetResolvedListInfoImpl(cursor, info))
		{
			cursor.GoToParent();
			return false;
		}

		if (!cursor.GoToNextNamedSibling())
		{
			cursor.GoToParent();
			return false;
		}

		if (cursor.CurrentNode().GetType() != NodeName::kIdentifier)
		{
			cursor.GoToParent();
			return false;
		}

		info.identifiers.emplace_back(cursor.CurrentNode().GetText());

		cursor.GoToParent();

		return true;
	}
	else
	{
		if (node.GetType() == NodeName::kFunctionCall)
		{
			if (!cursor.GoToFirstChild())
			{
				return false;
			}

			info.flags |= ResolvedListInfo::FunctionCall;

			const bool success = GetResolvedListInfoImpl(cursor, info);

			cursor.GoToParent();

			return success;
		}
	}

	return false;
}

ResolvedListInfo GetResolvedListInfo(TreeCursor& cursor)
{
	ResolvedListInfo info{};

	if (!GetResolvedListInfoImpl(cursor, info))
	{
		info.identifiers.clear();
		info.indexExprTypes.clear();
		info.flags = 0;
	}

	return info;
}

uint32_t GetSymTypeForListNode(TreeCursor& cursor, LuaSourceDocumentParseContext& ctx)
{
	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kVariableList || 
		   node.GetType() == NodeName::kExpressionList);

	auto resolvedInfo = GetResolvedListInfo(cursor);

	if (resolvedInfo.identifiers.empty())
	{
		return kInvalidLuaTypeId;
	}

	assert(resolvedInfo.identifiers.size() == resolvedInfo.indexExprTypes.size() - 1);

	const auto rootIdent = resolvedInfo.identifiers[0];
	assert(!rootIdent.empty());

	const size_t symIndex = GetSymIndexForIdentifier(
		rootIdent, node.GetSourceLocation(), ctx);

	if (symIndex >= ctx.symbols.Size())
	{
		return kInvalidLuaTypeId;
	}

	auto symType = ctx.symbols.GetView<&Sym::type>(symIndex);

	if (resolvedInfo.IsSingleIdentifier() || symType == kInvalidLuaTypeId)
	{
		return symType;
	}

	if (resolvedInfo.MaybeEnum())
	{
		// try to get type immediately from userTypeInfo if lhs is actually just an enum
		if (auto dataIdx = ctx.userTypeInfo.GetDataIndex(rootIdent); 
			dataIdx.IsValid())
		{
			return ctx.userTypeInfo.GetUserTypeId(dataIdx);
		}
	}

	// not an enum but a single or multi-level index expression
	for (size_t i = 1; i < resolvedInfo.identifiers.size(); ++i)
	{
		if (symType == kInvalidLuaTypeId)
		{
			return kInvalidLuaTypeId;
		}

		const auto rhsIdent = resolvedInfo.identifiers[i];
		const auto idxExprType = resolvedInfo.indexExprTypes[i - 1];

		assert(!rhsIdent.empty());

		const auto dataIdx = ctx.userTypeInfo.GetDataIndex(symType);
		if (!dataIdx.IsValid())
		{
			return kInvalidLuaTypeId;
		}

		const auto& fieldNames = ctx.userTypeInfo.GetUserTypeFieldNames(dataIdx);
		const auto& fieldKinds = ctx.userTypeInfo.GetUserTypeFieldMemberKinds(dataIdx);
		const auto& fieldResults = ctx.userTypeInfo.GetUserTypeFieldResultTypeIds(dataIdx);

		// for lhs object type, find its member being referenced by rhs and 
		// get its result type
		symType = kInvalidLuaTypeId;
		for (size_t j = 0; j < fieldNames.size(); ++j)
		{
			if (fieldNames[j] != rhsIdent)
			{
				continue;
			}

			if (!MemberKindMatchesIndexExprType(fieldKinds[j], idxExprType))
			{
				return kInvalidLuaTypeId;
			}
			
			symType = fieldResults[i];
			break;
		}
	}

	return symType;
}



} // unnamed

void LuaSourceDocumentParseContext::AddError(const TreeNode& node, std::string_view msg)
{
	errors.emplace_back(std::string{ msg }, node.GetSourceLocation());
}

void LuaSourceDocumentParseContext::AddError(const TreeCursor& cursor, std::string_view msg)
{
	AddError(cursor.CurrentNode(), msg);
}

} // ui::lua

#endif