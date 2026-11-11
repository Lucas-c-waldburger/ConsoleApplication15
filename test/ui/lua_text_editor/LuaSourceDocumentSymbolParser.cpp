#include "LuaSourceDocumentSymbolParser.h"

#if IMGUI_ENABLED
#include "TreeCursor.h"

namespace ui::lua {

namespace {

using Sym = LuaSourceDocumentSymbol;

struct TypedIdentifierInfo
{
	Sym symbol;
	std::string_view typeText;
	SourceLocation typeSpan;
	TreeCursor rhsCursor;

	bool HasTypeHint() const noexcept
	{
		return !typeText.empty() && typeSpan.start < typeSpan.end;
	}

	bool HasRhs() const noexcept
	{
		return rhsCursor && rhsCursor.CurrentNode().GetType() == NodeName::kExpressionList;
	}
};

//struct TypedIdentifierParseResult
//{
//	Sym symbol;
//	std::string_view typeText;
//	SourceLocation typeSpan;
//
//};

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
	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kVariableDeclaration);

	const auto nodeLoc = node.GetSourceLocation();

	std::optional<TypedIdentifierInfo> info{std::in_place};

	const size_t varDeclLocEnd = node.GetSourceLocation().end;

	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(node, "Failed to go to first child of variable_declaration node");
		return std::nullopt;
	}

	size_t depth = 1;

	info->symbol.kind = GetSymKindAndAdvance(cursor, ctx.scopeTree);
	if (info->symbol.kind == Sym::Kind::Invalid)
	{
		ctx.AddError(cursor, "Failed to go to node after local/global");
		cursor.GoToParent(depth);

		return std::nullopt;
	}

	info->symbol.scope = {
		node.GetSourceLocation().end + 1,
		(info->symbol.kind == Sym::Kind::UnscopedGlobal
			? ctx.scopeTree.GetDocumentEnd()
			: ctx.scopeTree.GetCurrentScopeLocation().end)
	};

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

	info->symbol.name = node.GetText();
	if (info->symbol.name.empty())
	{
		ctx.AddError(node, "identifier text was empty");

		cursor.GoToParent(depth);
		return std::nullopt;
	}

	info->symbol.location = node.GetSourceLocation();

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
	info->symbol.kind = Sym::Kind::Local;

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

	info->symbol.name = node.GetText();
	if (info->symbol.name.empty())
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
	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kParameters);

	const SourceLocation paramScope{
		.start = node.GetSourceLocation().end + 1,
		.end = ctx.scopeTree.GetCurrentScopeLocation().end
	};

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
		node = cursor.CurrentNode();
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
			auto& param = params.emplace_back(std::move(*paramInfo));
			param.symbol.scope = paramScope;
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

struct ExpressionListInfo
{
	static constexpr std::string_view kFunctionCallIdentifier = "*__fn_call";
	static constexpr IndexExpressionType kInvalidIndexExprType =
		std::numeric_limits<IndexExpressionType>::max();

	std::vector<std::string_view> identifiers;
	std::vector<IndexExpressionType> indexExprTypes;

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

	bool Empty() const noexcept
	{
		if (identifiers.empty())
		{
			assert(indexExprTypes.empty());
			return true;
		}
		return false;
	}
};

uint32_t GetResolvedExpressionListType(TreeCursor& cursor, LuaSourceDocumentParseContext& ctx,
									   IndexExpressionType currentIdxExpr, uint32_t currentType)
{
	auto node = cursor.CurrentNode();
	if (node.GetType() == NodeName::kIdentifier)
	{
		if (currentType == kInvalidLuaTypeId)
		{
			const size_t symIndex = GetSymIndexForIdentifier(
				node.GetText(), node.GetSourceLocation(), ctx);

			return (symIndex < ctx.symbols.Size())
				? ctx.symbols.GetView<&Sym::type>(symIndex)
				: kInvalidLuaTypeId;
		}
		else
		{
			if (currentIdxExpr == IndexExpressionType::Dot)
			{

			}
		}
	}
}

bool GetExpressionListInfoImpl(TreeCursor& cursor, ExpressionListInfo& info, 
							 LuaSourceDocumentParseContext& ctx)
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

		if (!GetExpressionListInfoImpl(cursor, info, ctx))
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
	else if (node.GetType() == NodeName::kFunctionCall)
	{
		info.identifiers.emplace_back();
		//info.indexExprTypes.emplace_back(ExpressionListInfo::kFunctionExprType);

		if (!cursor.GoToFirstChild())
		{
			return false;
		}

		const bool success = GetExpressionListInfoImpl(cursor, info, ctx);

		cursor.GoToParent();

		return success;
	}
	else
	{
		info.identifiers.emplace_back(node.GetType());

		return true;
	}

	return false;
}

ExpressionListInfo GetExpressionListInfo(TreeCursor& cursor, 
										 LuaSourceDocumentParseContext& ctx)
{
	assert(cursor.CurrentNode().GetType() == NodeName::kExpressionList);

	ExpressionListInfo info{};

	if (!GetExpressionListInfoImpl(cursor, info, ctx))
	{
		info.identifiers.clear();
		info.indexExprTypes.clear();
	}

	assert(cursor.CurrentNode().GetType() == NodeName::kExpressionList);

	return info;
}

size_t GetFunctionCallScopeIndex(std::string_view fnName,
								 const LuaSourceDocumentScopeTree& scopeTree)
{
	if (!scopeTree.CurrentValid() || fnName.empty())
	{
		return std::numeric_limits<size_t>::max();
	}

	for (size_t i = 0; i < scopeTree.names.size(); ++i)
	{
		if (fnName == scopeTree.names[i])
		{
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

uint32_t GetFunctionScopeReturnType(std::string_view fnName,
									const LuaSourceDocumentScopeTree& scopeTree)
{
	const size_t fnScope = GetFunctionCallScopeIndex(fnName, scopeTree);
	if (fnScope < scopeTree.Size())
	{
		return scopeTree.returns[fnScope];
	}

	return kInvalidLuaTypeId;
}

uint32_t GetSymTypeForExpressionListInfo(ExpressionListInfo& listInfo, 
										 SourceLocation listLoc,
										 bool isFunctionCall,
										 LuaSourceDocumentParseContext& ctx)
{
	assert(!listInfo.Empty());
	assert(listInfo.identifiers.size() == listInfo.indexExprTypes.size() - 1);

	auto rootIdent = listInfo.identifiers[0];
	if (rootIdent.empty())
	{
		return kInvalidLuaTypeId;
	}

	if (listInfo.IsSingleIdentifier())
	{
		if (isFunctionCall)
		{
			return GetFunctionScopeReturnType(rootIdent, ctx.scopeTree);
		}
		else if (rootIdent == NodeName::kNumber)
		{
			return kNativeNumberLuaTypeId;
		}
		else if (rootIdent == NodeName::kBoolean)
		{
			return kNativeBooleanLuaTypeId;
		}
		else if (rootIdent == NodeName::kString)
		{
			return kNativeStringLuaTypeId;
		}
		else if (rootIdent == NodeName::kTableConstructor)
		{
			return kNativeTableLuaTypeId;
		}
		else
		{
			const size_t symIndex = GetSymIndexForIdentifier(rootIdent, listLoc, ctx);

			return (symIndex < ctx.symbols.Size())
				? ctx.symbols.GetView<&Sym::type>(symIndex)
				: kInvalidLuaTypeId;
		}
	}

	// has at least one index expression
	if (listInfo.MaybeEnum())
	{
		// try to get type immediately from userTypeInfo if lhs is actually just an enum
		if (auto dataIdx = ctx.userTypeInfo.GetDataIndex(rootIdent);
			dataIdx.IsValid())
		{
			return ctx.userTypeInfo.GetUserTypeId(dataIdx);
		}
	}

	// not an enum, is def a single or multi-level index expression
	// get the root identifier's sym type and go from there
	size_t symIndex = GetSymIndexForIdentifier(rootIdent, listLoc, ctx);
	if (symIndex >= ctx.symbols.Size())
	{
		return kInvalidLuaTypeId;
	}

	auto symType = ctx.symbols.GetView<&Sym::type>(symIndex);

	for (size_t i = 1; i < listInfo.identifiers.size(); ++i)
	{
		if (symType == kInvalidLuaTypeId)
		{
			return kInvalidLuaTypeId;
		}

		const auto rhsIdent = listInfo.identifiers[i];
		const auto idxExprType = listInfo.indexExprTypes[i - 1];

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

void AddGlobalSymsForFunctionCall(std::string_view fnName,
								  SourceLocation callLoc,
								  LuaSourceDocumentParseContext& ctx)
{
	const size_t fnScopeIdx = GetFunctionCallScopeIndex(fnName, ctx.scopeTree);
	if (fnScopeIdx >= ctx.scopeTree.Size())
	{
		return;
	}

	const auto& scopeLoc = ctx.scopeTree.locations[fnScopeIdx];

	// find all symbols inside the called function's declaration
	// if any were scoped globals, add them as unscoped globals at this location
	const size_t origSymbolsSize = ctx.symbols.Size();
	for (size_t i = 0; i < origSymbolsSize; ++i)
	{
		const auto [symName, symLoc, _, symType, symKind] =
			ctx.symbols.GetView(i);

		if (symLoc.start >= scopeLoc.end)
		{
			break;
		}
		if (symLoc.start < scopeLoc.start || symKind != Sym::Kind::ScopedGlobal)
		{
			continue;
		}

		ctx.symbols.PushBack(Sym{
			.name = symName,
			.location = callLoc,
			.scope = { callLoc.end + 1, ctx.scopeTree.GetDocumentEnd() },
			.type = symType,
			.kind = Sym::Kind::UnscopedGlobal
		});
	}
}

void AddSymForVariableDeclaration(TreeCursor& cursor, LuaSourceDocumentParseContext& ctx)
{
	auto typedIdentInfo = ParseVariableDeclaration(cursor, ctx);
	if (!typedIdentInfo)
	{
		return;
	}

	if (typedIdentInfo->HasTypeHint())
	{
		ctx.eraseLocations.emplace_back(typedIdentInfo->typeSpan);

		if (auto dataIdx = ctx.userTypeInfo.GetDataIndex(typedIdentInfo->typeText);
			dataIdx.IsValid())
		{
			typedIdentInfo->symbol.type = ctx.userTypeInfo.GetUserTypeId(dataIdx);
		}
	}

	std::string_view fnCallText;
	SourceLocation fnCallLoc;

	if (typedIdentInfo->HasRhs())
	{
		if (!typedIdentInfo->rhsCursor.GoToFirstChild())
		{
			ctx.AddError(typedIdentInfo->rhsCursor, 
						"Failed to go to first child of expression_list node");

			ctx.symbols.PushBack(std::move(typedIdentInfo->symbol));
			return;
		}

		auto node = typedIdentInfo->rhsCursor.CurrentNode();
		if (node.GetType() == NodeName::kFunctionCall)
		{
			if (!typedIdentInfo->rhsCursor.GoToFirstChild())
			{
				ctx.AddError(typedIdentInfo->rhsCursor, 
					"Failed to go to first child of function_call node");

				ctx.symbols.PushBack(std::move(typedIdentInfo->symbol));
				return;
			}

			fnCallText = node.GetText();
			fnCallLoc = node.GetSourceLocation();
		}

		const auto exprListLoc = typedIdentInfo->rhsCursor.CurrentNode().GetSourceLocation();

		auto exprListInfo = GetExpressionListInfo(typedIdentInfo->rhsCursor, ctx);
		if (exprListInfo.Empty())
		{
			ctx.AddError(typedIdentInfo->rhsCursor, 
				"Failed to get expression_list info for rhs of variable_declaration");

			ctx.symbols.PushBack(std::move(typedIdentInfo->symbol));
			return;
		}

		const auto evaluatedType = GetSymTypeForExpressionListInfo(
			exprListInfo, exprListLoc, !fnCallText.empty(), ctx
		);

		if (evaluatedType != kInvalidLuaTypeId)
		{
			if (typedIdentInfo->symbol.type != kInvalidLuaTypeId &&
				typedIdentInfo->symbol.type != evaluatedType)
			{
				ctx.AddError(typedIdentInfo->rhsCursor,
					"Assigned type differs from hinted type");
			}

			typedIdentInfo->symbol.type = evaluatedType;
		}
	}

	ctx.symbols.PushBack(std::move(typedIdentInfo->symbol));

	if (!fnCallText.empty())
	{
		AddGlobalSymsForFunctionCall(fnCallText, fnCallLoc, ctx);
	}
}

void AddSymsForParameters(TreeCursor& cursor, LuaSourceDocumentParseContext& ctx)
{
	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kParameters);

	const SourceLocation paramScope{
		.start = node.GetSourceLocation().end + 1,
		.end = ctx.scopeTree.GetCurrentScopeLocation().end
	};

	if (!cursor.GoToFirstChild())
	{
		return;
	}

	if (cursor.CurrentNode().GetType() != "(")
	{
		ctx.AddError(cursor,
			std::format("Expected '(' node, got {}", cursor.CurrentNode().GetType()));

		cursor.GoToParent();
		return;
	}

	size_t posCounter = 0;
	while (cursor.GoToNextSibling())
	{
		node = cursor.CurrentNode();
		if (node.GetType() == ")")
		{
			break;
		}

		if (node.GetType() != NodeName::kTypedIdentifier)
		{
			ctx.AddError(node,
				std::format("Expected typed_identifier node, got {}", node.GetType()));

			++posCounter;
			continue;
		}

		auto paramInfo = ParseSingleParameter(cursor, ctx);
		if (!paramInfo)
		{
			ctx.AddError(cursor,
				std::format("Failed to parse parameter at position {}", posCounter));

			++posCounter;
			continue;
		}

		if (paramInfo->HasTypeHint())
		{
			if (paramInfo->typeText == NodeName::kNumber)
			{
				paramInfo->symbol.type = kNativeNumberLuaTypeId;
			}
			else if (paramInfo->typeText == NodeName::kBoolean)
			{
				paramInfo->symbol.type = kNativeBooleanLuaTypeId;
			}
			else if (paramInfo->typeText == NodeName::kString)
			{
				paramInfo->symbol.type = kNativeStringLuaTypeId;
			}
			else if (node.GetType() == NodeName::kTableConstructor)
			{
				paramInfo->symbol.type = kNativeTableLuaTypeId;
			}
			else if (auto dataIdx = ctx.userTypeInfo.GetDataIndex(paramInfo->typeText);
					dataIdx.IsValid())
			{
				paramInfo->symbol.type = ctx.userTypeInfo.GetUserTypeId(dataIdx);
			}
		}

		paramInfo->symbol.scope = paramScope;

		ctx.symbols.PushBack(std::move(paramInfo->symbol));

		++posCounter;
	}
}

bool IsFunctionScope(std::string_view scopeName)
{
	return !scopeName.empty() && scopeName != kBlockScopeName;
}

size_t GetCurrentParentFunctionScopeIndex(LuaSourceDocumentScopeTree& scopeTree)
{
	if (!scopeTree.CurrentValid())
	{
		return std::numeric_limits<size_t>::max();
	}

	const size_t origScopeIdx = scopeTree.current;
	size_t result = std::numeric_limits<size_t>::max();
	do
	{
		auto scopeName = scopeTree.GetCurrentScopeName();
		if (IsFunctionScope(scopeName))
		{
			result = scopeTree.current;
			break;
		}

	} while (scopeTree.GoToParent());

	scopeTree.SetCurrent(origScopeIdx);

	return result;
}

void ParseFunctionDeclarations(TreeCursor& cursor, LuaSourceDocumentParseContext& ctx)
{
	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(cursor, "Failed to go to first child of root node");
	}


}

void AddScopeReturnType(TreeCursor& cursor, LuaSourceDocumentParseContext& ctx)
{
	//assert(cursor.CurrentNode().GetType() == NodeName::kReturnStatement);

	//// if inside inner fn block, get the function scope that it's inside
	//const size_t parentFnScopeIdx = GetCurrentParentFunctionScopeIndex(ctx.scopeTree);
	//if (parentFnScopeIdx >= ctx.scopeTree.Size())
	//{
	//	return;
	//}

	//if (cursor.CurrentNode().GetNamedChildCount() != 1)
	//{
	//	return;
	//}
	//if (!cursor.GoToFirstNamedChild())
	//{
	//	return;
	//}

	//// return_statement -> expression_list -> expression
	//auto node = cursor.CurrentNode();
	//if (node.GetType() == NodeName::kExpressionList)
	//{
	//	const auto exprNodeLoc = node.GetSourceLocation();

	//	auto exprListInfo = GetExpressionListInfo(cursor, ctx);
	//	if (exprListInfo.Empty())
	//	{
	//		return;
	//	}

	//	auto retType = GetSymTypeForExpressionListInfo(
	//		exprListInfo, exprNodeLoc, false, ctx
	//	);

	//	auto exprNode = cursor.CurrentNode();

	//	const uint32_t retType = GetSymTypeForRhsExpression(
	//		cursor, exprNode.GetSourceLocation(), ctx);

	//	if (exprNode.GetType() == NodeName::kFunctionCall)
	//	{
	//		AddGlobalSymbolsForFunctionCall(cursor, ctx);
	//	}

	//	ctx.scopeTree.returns[parentFnScopeIdx] = retType;

	//	cursor.GoToParent();
	//}

	//cursor.GoToParent();
}

void AddSymForFunctionDeclaration(TreeCursor& cursor, LuaSourceDocumentParseContext& ctx)
{
	assert(cursor.CurrentNode().GetType() == NodeName::kFunctionDeclaration);

	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(cursor, "Failed to go to first child of function_declaration");
	}

	Sym sym{
		.kind = GetSymKindAndAdvance(cursor, ctx.scopeTree)
	};

	if (sym.kind == Sym::Kind::Invalid)
	{
		ctx.AddError(cursor, "Failed to go to node after local/global");

		cursor.GoToParent();
		return;
	}

	auto node = cursor.CurrentNode();
	if (node.GetType() != kFunctionName)
	{
		ctx.AddError(cursor, std::format(
			"Expected node type {}, got {}", kFunctionName, node.GetType()));

		cursor.GoToParent();
		return;
	}

	if (!cursor.GoToNextSibling())
	{
		ctx.AddError(cursor, "Failed to go to node after function");

		cursor.GoToParent();
		return;
	}

	node = cursor.CurrentNode();
	if (node.GetType() == NodeName::kIdentifier)
	{
		// free function
	}
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

bool ParseLuaSourceDocumentSymbols(TreeCursor& cursor, LuaSourceDocumentParseContext& ctx)
{
	/*if (!cursor.GoToFirstChild())
	{
		return false;
	}

	do
	{
		auto node = cursor.CurrentNode();

		while (NodeEndsCurrentScope(node, ctx.scopeTree))
		{
			ctx.scopeTree.GoToParent();
		}

		bool handled = false;

		if (NodeStartsNewScope(node))
		{
			ctx.scopeTree.PushScope(cursor);
		}

		if (node.GetType() == NodeName::kFunctionDeclaration)
		{

		}
		else if (node.GetType() == NodeName::kVariableDeclaration)
		{
			AddSymForVariableDeclaration(cursor, ctx);
			handled = true;
		}
		else if (node.GetType() == NodeName::kFunctionCall)
		{
			AddGlobalSymsForFunctionCall(node.GetText(), node.GetSourceLocation(), ctx);
		}
		else if (node.GetType() == NodeName::kAssignmentStatement)
		{
			AddSymForAssignmentStatement(cursor, ctx);
			handled = true;
		}
		else if (node.GetType() == NodeName::kReturnStatement)
		{
			AddScopeReturnType(cursor, ctx);
		}
		else if (node.GetType() == NodeName::kParameters)
		{
			AddFunctionParameterSymbols(cursor, ctx);
		}

		if (!handled && node.GetChildCount() > 0)
		{
			ParseLuaSourceDocumentSymbolsImpl(cursor, ctx);
		}

	} while (cursor.GoToNextSibling());

	cursor.GoToParent();*/

	return true;
}

} // ui::lua

#endif