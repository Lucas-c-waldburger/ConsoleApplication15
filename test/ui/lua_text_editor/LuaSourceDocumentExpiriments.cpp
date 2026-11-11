#include "LuaSourceDocumentExpiriments.h"

#if IMGUI_ENABLED

namespace ui {

namespace {

constexpr bool MemberKindMatchesIndexExprType(LuaUsertypeMemberKind kind,
											  IndexExpressionType type) noexcept
{
	return (type == IndexExpressionType::Dot && IsMemberKindField(kind)) ||
		(type == IndexExpressionType::Method && IsMemberKindFunction(kind));
}

std::optional<ScopeKind> GetScopeKind(const TreeNode& node)
{
	if (node.GetType() == NodeName::kChunk)
	{
		return ScopeKind::Chunk;
	}
	if (node.GetType() == NodeName::kFunctionDeclaration ||
	    node.GetType() == NodeName::kFunctionDefinition)
	{
		return ScopeKind::Function;
	}
	if (node.GetType() == NodeName::kBlock)
	{
		return ScopeKind::Block;
	}

	return std::nullopt;
}

bool IsScopeNode(const TreeNode& node)
{
	return GetScopeKind(node) != ScopeKind::Invalid;
}

void PushScope(TreeCursor& cursor, LuaSourceDocParseContext& ctx, ScopeKind kind)
{
	auto& tree = ctx.scopeTree;

	const size_t idx = tree.Size();
	if (idx > 0)
	{
		assert(tree.current < tree.scopes.size());
		tree.scopes[tree.current].children.emplace_back(idx);

		if (tree.symbolRanges[tree.current].min > ctx.symbols.Size())
		{
			tree.symbolRanges[tree.current].min = ctx.symbols.Size();
		}
		tree.symbolRanges[tree.current].max = ctx.symbols.Size();
	}

	auto node = cursor.CurrentNode();
	assert(node);

	tree.kinds.emplace_back(kind);
	tree.locations.emplace_back(node.GetSourceLocation());
	tree.contents.emplace_back(node);
	tree.resultTypes.emplace_back(kInvalidLuaTypeId);
	tree.classes.emplace_back(GetScopeClass(cursor, tree));
	tree.symbolRanges.emplace_back(ctx.symbols.Size() + 1, ctx.symbols.Size() + 1);

	auto& newScope = tree.scopes.emplace_back();
	newScope.index = idx;
	newScope.parent = idx > 0 ? tree.current : Null<size_t>();

	tree.current = idx;
}

bool PopScope(ScopeTree& tree)
{
	if (tree.current < tree.scopes.size() && 
		tree.scopes[tree.current].parent < tree.scopes.size())
	{
		tree.current = tree.scopes[tree.current].parent;
		return true;
	}
	return false;
}

bool NodeEndsCurrentScope(const TreeNode& node, const ScopeTree& tree)
{
	return tree.current > 0 &&
		   tree.current < tree.locations.size() &&
		   node.GetStartPos() >= tree.locations[tree.current].end;
}

//void ParseScopeTreeImpl(TreeCursor& cursor, ScopeTree& tree, bool enteredFunction)
//{
//	do
//	{
//		auto node = cursor.CurrentNode();
//
//		while (NodeEndsCurrentScope(node, tree))
//		{
//			PopScope(tree);
//		}
//
//		const auto kind = GetScopeKind(node);
//		if (kind == ScopeKind::Invalid)		  
//		{
//			continue;
//		}
//
//		if (!(kind == ScopeKind::Block && enteredFunction))
//		{
//			PushScope(node, tree, kind);
//		}
//
//		if (cursor.GoToFirstChild())
//		{
//			ParseScopeTreeImpl(cursor, tree, kind == ScopeKind::Function);
//
//			cursor.GoToParent();
//		}
//
//	} while (cursor.GoToNextSibling());
//}

size_t GetSymbolVisibilityEnd(ScopeClass scopeClass, const LuaSourceDocParseContext& ctx)
{
	return scopeClass == ScopeClass::UnscopedGlobal
		? ctx.scopeTree.GetDocumentEnd()
		: ctx.scopeTree.GetCurrentScopeEnd();
}

ScopeClass GetGlobalScopeClass(const ScopeTree& tree)
{
	const auto fnScope = tree.FindEnclosingFunctionScope(tree.current);

	return fnScope != Null<ScopeID>()
		? ScopeClass::ScopedGlobal
		: ScopeClass::UnscopedGlobal;
}

std::optional<ScopeClass> GetScopeClassForDeclaration(TreeCursor& cursor, 
													  const ScopeTree& tree)
{
	auto node = cursor.CurrentNode();
	const auto nodeType = node.GetType();

	std::optional<ScopeClass> scopeClass;

	if (nodeType == kLocalName)
	{
		scopeClass = ScopeClass::Local;
	}
	else if (nodeType == kGlobalName)
	{
		scopeClass = GetGlobalScopeClass(tree);
	}

	return scopeClass;
}


ScopeClass GetScopeClass(TreeCursor& cursor, const ScopeTree& tree)
{
	if (cursor.GoToFirstChild())
	{
		auto scopeClass = GetScopeClassForDeclaration(cursor, tree);

		cursor.GoToParent();

		if (scopeClass)
		{
			return *scopeClass;
		}
	}

	return ScopeClass::UnscopedGlobal;
}

TreeCursor GetExpressionListCursor(TreeCursor& cursor, LuaSourceDocParseContext& ctx)
{
	auto node = cursor.CurrentNode();
	if (node.GetType() != NodeName::kExpressionList)
	{
		ctx.AddError(node,
			std::format("Expected expression_list node, got {}", node.GetType()));

		return {};
	}

	if (node.GetNamedChildCount() != 1)
	{
		ctx.AddError(node, "expression_list node did not have one named child");

		return {};
	}

	if (!cursor.GoToFirstNamedChild())
	{
		ctx.AddError(cursor, "failed to go to first named child of expression_list node");

		return {};
	}

	auto branched = cursor.Branch();

	cursor.GoToParent();

	return branched;
}

void ParseVariableDeclaration(TreeCursor& cursor, LuaSourceDocParseContext& ctx)
{
	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kVariableDeclaration);

	size_t symVisStart = node.GetEndPos();

	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(node, "Failed to go to first child of variable_declaration node");
		return;
	}

	size_t depth = 1;

	const auto scopeClass = GetScopeClassForDeclaration(cursor, ctx.scopeTree);
	if (!scopeClass)
	{
		ctx.AddError(cursor, std::format("Expected local or global type for "
			"first node of declaration, got '{}'", cursor.CurrentNode().GetType()));

		cursor.GoToParent(depth);
		return;
	}

	if (!cursor.GoToNextSibling())
	{
		ctx.AddError(cursor, "Failed to go to node after local/global");

		cursor.GoToParent();
		return;
	}

	node = cursor.CurrentNode();

	const size_t symVisEnd = GetSymbolVisibilityEnd(*scopeClass, ctx);

	Symbol sym{
		.location = node.GetSourceLocation(),
		.visibility = { symVisStart, symVisEnd },
		.scopeClass = *scopeClass,
		.scopeId = ctx.scopeTree.current
	};

	bool hasAssignStmt = false;

	if (node.GetType() == NodeName::kVariableList)
	{
		if (node.GetChildCount() != 1)
		{
			ctx.AddError(cursor, "variable_list node did not have one child. "
				"Only single-variable assignments are supported");

			cursor.GoToParent(depth);
			return;
		}

		if (!cursor.GoToFirstChild())
		{
			ctx.AddError(cursor, 
				"failed to go to first child of variable_list node");

			cursor.GoToParent();
			return;
		}
		++depth;

		if (cursor.CurrentNode().GetType() != NodeName::kTypedIdentifier)
		{
			ctx.AddError(cursor,
				"first child of variable_list node was not typed_identifier");

			cursor.GoToParent(depth);
		}
	}
	else if (node.GetType() == NodeName::kAssignmentStatement)
	{
		hasAssignStmt = true;

		if (node.GetNamedChildCount() != 2)
		{
			ctx.AddError(cursor,
				"assignment_statement node did not have two named children");

			cursor.GoToParent(depth);
		}
		if (!cursor.GoToFirstChild())
		{
			ctx.AddError(cursor,
				"failed to go to first child of assignment_statement node");
			cursor.GoToParent(depth);

			return;
		}
		++depth;

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kVariableList)
		{
			ctx.AddError(node,
				"first child of assignment_statement node was not variable_list");

			cursor.GoToParent(depth);
		}

		if (node.GetChildCount() != 1)
		{
			ctx.AddError(node,
				"variable_list node did not have one child");

			cursor.GoToParent(depth);
		}

		if (!cursor.GoToFirstChild())
		{
			ctx.AddError(cursor,
				"failed to go to first child of variable_list node");

			cursor.GoToParent(depth);
		}
		++depth;

		if (cursor.CurrentNode().GetType() != NodeName::kTypedIdentifier)
		{
			ctx.AddError(cursor,
				"first child of variable_list node was not typed_identifier");

			cursor.GoToParent(depth);
		}
	}
	else
	{
		ctx.AddError(node,
			"variable_declaration node did not have variable_list "
			"or assignment_statement child");

		cursor.GoToParent(depth);
	}

	assert(cursor.CurrentNode().GetType() == NodeName::kTypedIdentifier);

	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(cursor, "failed to go to first child of typed_identifier node");

		cursor.GoToParent(depth);
	}
	++depth;

	node = cursor.CurrentNode();
	if (node.GetType() != NodeName::kIdentifier)
	{
		ctx.AddError(node, "first child of typed_identifier node was not identifier");

		cursor.GoToParent(depth);
	}

	sym.name = node.GetText();
	if (sym.name.empty())
	{
		ctx.AddError(node, "identifier text was empty");

		cursor.GoToParent(depth);
	}

	std::string_view typeText;
	SourceLocation typeSpan{};

	if (cursor.GoToNextSibling()) // check for type hint
	{
		node = cursor.CurrentNode();
		if (node.GetType() != ":")
		{
			ctx.AddError(node,
				std::format("Expected ':' node after identifier, got {}", node.GetType()));

			cursor.GoToParent(depth);
		}

		typeSpan.start = node.GetSourceLocation().start;

		if (!cursor.GoToNextSibling())
		{
			ctx.AddError(cursor, "failed to go to next sibling after ':' node");

			cursor.GoToParent(depth);
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kIdentifier)
		{
			ctx.AddError(node,
				"second named child of typed_identifier node was not identifier");

			cursor.GoToParent(depth);
		}

		typeText = node.GetText();
		if (typeText.empty())
		{
			ctx.AddError(node, "identifier text was empty");

			cursor.GoToParent(depth);
		}

		typeSpan.end = node.GetSourceLocation().end;
	}

	if (!hasAssignStmt)
	{
		const auto symId = ctx.symbols.PushBack(sym);

		const size_t declIdx = ctx.declarations.AddEntry();
		ctx.declarations.typeText[declIdx] = typeText;
		ctx.declarations.typeSpans[declIdx] = typeSpan;
		ctx.declarations.symbolIds[declIdx] = symId;

		cursor.GoToParent(depth);
		return;
	}

	cursor.GoToParent(2); // back to variable_list
	----depth;

	if (!cursor.GoToNextNamedSibling()) // to expression_list
	{
		ctx.AddError(cursor, "failed to go to expression_list node after variable_list");

		cursor.GoToParent(depth);
		return;
	}

	auto rhsCursor = GetExpressionListCursor(cursor, ctx);
	if (!rhsCursor)
	{
		cursor.GoToParent(depth);
		return;
	}

	const auto symId = ctx.symbols.PushBack(sym);

	const size_t declIdx = ctx.declarations.AddEntry();
	ctx.declarations.typeText[declIdx] = typeText;
	ctx.declarations.typeSpans[declIdx] = typeSpan;
	ctx.declarations.symbolIds[declIdx] = symId;
	ctx.declarations.rhsCursors[declIdx] = std::move(rhsCursor);

	cursor.GoToParent(depth);

	return;
}

// RESOLVE VARIABLE DECLARATIONS

SymbolID GetExistingSymbolID(std::string_view ident, SourceLocation loc,
							 LuaSourceDocParseContext& ctx)
{
	for (size_t i = ctx.symbols.Size(); i-- > 0;)
	{
		const auto [symName, symVis] =
			ctx.symbols.GetView<&Symbol::name, &Symbol::visibility>(i);

		if (ident == symName && loc.Inside(symVis))
		{
			return i;
		}
	}

	return Null<SymbolID>();
}

SymbolID GetExistingSymbolID(std::string_view ident, SourceLocation loc,
							 SymbolSuperType expectedSuperType,
							 LuaSourceDocParseContext& ctx)
{
	for (size_t i = ctx.symbols.Size(); i-- > 0;)
	{
		const auto [symName, symVis, symSuper] =
			ctx.symbols.GetView<&Symbol::name, &Symbol::visibility, 
								&Symbol::super>(i);

		if (ident == symName && loc.Inside(symVis) && 
			symSuper == expectedSuperType)
		{
			return i;
		}
	}

	return Null<SymbolID>();
}

bool FillAssignmentStatementLhsDataImpl(TreeCursor& cursor, LuaSourceDocParseContext& ctx,
										std::vector<std::string_view>& idents,
										std::vector<IndexExpressionType>& exprTypes)
{
	auto node = cursor.CurrentNode();
	if (node.GetType() == NodeName::kIdentifier)
	{
		idents.emplace_back(node.GetText());

		return true;
	}
	else if (IsIndexExpressionNode(node))
	{
		exprTypes.emplace_back((node.GetType() == NodeName::kDotIndexExpression
			? IndexExpressionType::Dot
			: IndexExpressionType::Method));

		if (!cursor.GoToFirstChild())
		{
			ctx.AddError(cursor, "Failed to go to first child of index expression node");
			return false;
		}

		if (!FillAssignmentStatementLhsDataImpl(cursor, ctx, idents, exprTypes))
		{
			cursor.GoToParent();
			return false;
		}

		if (!cursor.GoToNextNamedSibling())
		{
			ctx.AddError(cursor, "Failed to go to next sibling of index expression node");

			cursor.GoToParent();
			return false;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kIdentifier)
		{
			ctx.AddError(node, std::format("Expected next sibling of index "
				"expression node to be identifier, got '{}'", node.GetType()));

			cursor.GoToParent();
			return false;
		}

		idents.emplace_back(cursor.CurrentNode().GetText());

		cursor.GoToParent();

		return true;
	}
	else if (node.GetType() == NodeName::kFunctionCall)
	{
		if (!cursor.GoToFirstChild())
		{
			ctx.AddError(cursor, "Failed to go to first child of function_call node");
			return false;
		}

		const bool success = 
			FillAssignmentStatementLhsDataImpl(cursor, ctx, idents, exprTypes);

		cursor.GoToParent();

		return success;
	}

	return false;
}

bool FillAssignmentStatementLhsData(TreeCursor& cursor, LuaSourceDocParseContext& ctx,
									size_t assignStmtIdx)
{
	auto node = cursor.CurrentNode();
	assert(IsIndexExpressionNode(node));

	assert(assignStmtIdx < ctx.assignments.Size());

	auto& idents = ctx.assignments.identifiers[assignStmtIdx];
	auto& exprTypes = ctx.assignments.indexExprTypes[assignStmtIdx];

	return FillAssignmentStatementLhsDataImpl(cursor, ctx, idents, exprTypes);
}

void ParseAssignmentStatement(TreeCursor& cursor, LuaSourceDocParseContext& ctx)
{
	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kAssignmentStatement);

	const auto assignLoc = node.GetSourceLocation();

	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(node,
			"Failed to go to first child of assignment_statement node");

		return;
	}

	size_t depth = 1;

	node = cursor.CurrentNode();
	if (node.GetType() != NodeName::kVariableList)
	{
		ctx.AddError(node, std::format("Expected first child of "
			"assignement_statement node to be variable_list, got '{}'",
			node.GetType()));

		cursor.GoToParent(depth);
		return;
	}

	if (node.GetChildCount() != 1)
	{
		ctx.AddError(cursor, "variable_list node did not have one child. "
			"Only single-variable assignments are supported");
		
		cursor.GoToParent(depth);
		return;
	}

	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(node,
			"Failed to go to first child of variable_list node");

		return;
	}
	++depth;

	Symbol sym{
		.location = assignLoc,
		.scopeId = ctx.scopeTree.current
	};

	size_t assignStmtIdx = Null<size_t>();

	node = cursor.CurrentNode();
	if (node.GetType() == NodeName::kIdentifier)
	{
		// only single identifier might be new declaration
		const auto name = node.GetText();
		if (name.empty())
		{
			cursor.GoToParent(depth);
			return;
		}

		sym.name = name;

		SymbolID prevSymId = GetExistingSymbolID(name, node.GetSourceLocation(), ctx);
		if (prevSymId == Null<SymbolID>())
		{
			// no prev visible symbol with same name exists, must be declaration
			sym.scopeClass = GetGlobalScopeClass(ctx.scopeTree);
		}
		else
		{
			// prexisting symbol, inhert its scope class
			assert(prevSymId < ctx.symbols.Size());
			sym.scopeClass = ctx.symbols.GetView<&Symbol::scopeClass>(prevSymId);
		}

		const auto visEnd = GetSymbolVisibilityEnd(sym.scopeClass, ctx);

		sym.visibility = { sym.location.end, visEnd };
	}
	else if (IsIndexExpressionNode(node))
	{
		// not a new decl, get expr info
		assignStmtIdx = ctx.assignments.AddEntry();

		if (!FillAssignmentStatementLhsData(cursor, ctx, assignStmtIdx))
		{
			ctx.assignments.PopEntry();

			cursor.GoToParent(depth);
			return;
		}
	}
	else
	{
		ctx.AddError(node, std::format("Expected child of variable_list node "
			"to be an identifer or index expression node, got '{}'", node.GetType()));

		cursor.GoToParent(depth);
		return;
	}

	// get rhs cursor before handling if new decl
	cursor.GoToParent(); // back to variable_list
	--depth;

	const bool addedAssignStmt = assignStmtIdx < ctx.assignments.Size();
	if (!cursor.GoToNextNamedSibling()) // to expression_list
	{
		ctx.AddError(cursor, "failed to go to expression_list node after variable_list");

		if (addedAssignStmt)
		{
			ctx.assignments.PopEntry();
		}

		cursor.GoToParent(depth);
		return;
	}

	auto rhsCursor = GetExpressionListCursor(cursor, ctx);
	if (!rhsCursor)
	{
		if (addedAssignStmt)
		{
			ctx.assignments.PopEntry();
		}

		cursor.GoToParent(depth);
		return;
	}

	const auto symId = ctx.symbols.PushBack(sym);

	if (addedAssignStmt)
	{
		ctx.assignments.symbolIds[assignStmtIdx] = symId;
		ctx.assignments.rhsCursors[assignStmtIdx] = std::move(rhsCursor);
	}
	else
	{
		const size_t declIdx = ctx.declarations.AddEntry();
		ctx.declarations.symbolIds[declIdx] = symId;
		ctx.declarations.rhsCursors[declIdx] = std::move(rhsCursor);
	}

	cursor.GoToParent(depth);
}

void AddParameterSymbols(TreeCursor& cursor, LuaSourceDocParseContext& ctx,
						 ScopeID scopeId, SourceLocation paramVis)
{
	size_t paramCounter = 0;

	do
	{
		auto node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kTypedIdentifier)
		{
			ctx.AddError(node, std::format("Failed to parse parameter '{}': "
				"Expected typed_identifier node, got {}",
				paramCounter, node.GetType()));

			++paramCounter;
			continue;
		}

		if (!cursor.GoToFirstChild())
		{
			ctx.AddError(cursor, std::format("Failed to parse parameter '{}': "
				"failed to go to first child of typed_identifier node",
				paramCounter));

			++paramCounter;
			continue;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kIdentifier)
		{
			ctx.AddError(node, std::format("Failed to parse parameter '{}': "
				"first child of typed_identifier node was not identifier",
				paramCounter));

			++paramCounter;
			cursor.GoToParent();
			continue;
		}

		auto name = node.GetText();
		if (node.GetText().empty())
		{
			ctx.AddError(node, std::format("Failed to parse parameter '{}': "
				"parameter name was empty", paramCounter));

			++paramCounter;
			cursor.GoToParent();
			continue;
		}

		const size_t symIdx = ctx.symbols.PushBack({
			.name = name,
			.location = node.GetSourceLocation(),
			.visibility = paramVis,
			.super = SymbolSuperType::Parameter,
			.scopeClass = ScopeClass::Local,
			.scopeId = scopeId
			});

		const size_t declIdx = ctx.declarations.AddEntry();
		ctx.declarations.symbolIds[declIdx] = symIdx;

		if (!cursor.GoToNextNamedSibling()) // check for type hint
		{
			++paramCounter;
			cursor.GoToParent();
			continue;
		}

		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kIdentifier)
		{
			ctx.AddError(node, std::format("Error parsing parameter type '{}': "
				"Expected second named child of typed_identifier to be "
				"parameter, got '{}'", paramCounter, node.GetType()));

			++paramCounter;
			cursor.GoToParent();
			continue;
		}

		auto typeText = node.GetText();
		if (typeText.empty())
		{
			ctx.AddError(node, std::format("Error parsing parameter type '{}': "
				"type text was empty", paramCounter));

			++paramCounter;
			cursor.GoToParent();
			continue;
		}

		ctx.declarations.typeText[declIdx] = typeText;
		ctx.declarations.typeSpans[declIdx] = node.GetSourceLocation();

		++paramCounter;

	} while (cursor.GoToNextNamedSibling());
}

bool ParseFunction(TreeCursor& cursor, LuaSourceDocParseContext& ctx)
{
	auto node = cursor.CurrentNode();
	std::string_view fnNodeType = node.GetType();

	assert((fnNodeType == NodeName::kFunctionDeclaration ||
			fnNodeType == NodeName::kFunctionDefinition));

	const auto scopeId = ctx.scopeTree.current;
	assert(ctx.scopeTree.current < ctx.scopeTree.Size());

	Symbol sym{
		.location = ctx.scopeTree.locations[scopeId],
		.super = SymbolSuperType::Function,
		.scopeClass = ctx.scopeTree.classes[scopeId],
		.scopeId = scopeId
	};
	sym.visibility = { 
		sym.location.end, 
		GetSymbolVisibilityEnd(sym.scopeClass, ctx) 
	};

	if (!cursor.GoToFirstNamedChild()) // skip over local/global and 'function'
	{
		ctx.AddError(node, std::format(
			"Failed to go to first named child of {} node", fnNodeType));

		return true; // handled
	}

	size_t depth = 1;

	size_t assignStmtIdx = Null<size_t>();

	node = cursor.CurrentNode();
	if (node.GetType() == NodeName::kIdentifier)
	{
		sym.name = node.GetText();
		if (sym.name.empty())
		{
			ctx.AddError(node, "function identifier text was empty");

			cursor.GoToParent(depth);
			return true; // handled
		}
	}
	else if (IsIndexExpressionNode(node))
	{
		assignStmtIdx = ctx.assignments.AddEntry();

		// we wont add a cursor since we'll process function body below
		if (!FillAssignmentStatementLhsData(cursor, ctx, assignStmtIdx))
		{
			ctx.assignments.PopEntry();

			cursor.GoToParent(depth);
			return true; // handled
		}
	}
	else
	{
		ctx.AddError(node, std::format("Expected next sibling after 'function' "
			"to be an identifer or index expression node, got '{}'", node.GetType()));

		cursor.GoToParent(depth);
		return true; // handled
	}
	
	const bool addedAssignStmt = assignStmtIdx < ctx.assignments.Size();
	if (!cursor.GoToNextSibling())
	{
		ctx.AddError(node, "Failed to go to parameters node");

		if (addedAssignStmt)
		{
			ctx.assignments.PopEntry();
		}

		cursor.GoToParent(depth);
		return true; // handled
	}

	node = cursor.CurrentNode();
	if (node.GetType() != NodeName::kParameters)
	{
		ctx.AddError(node, std::format(
			"Expected parameters node, got '{}'", node.GetType()));

		if (addedAssignStmt)
		{
			ctx.assignments.PopEntry();
		}

		cursor.GoToParent(depth);
		return true; // handled
	}

	const SourceLocation paramVis{
		node.GetEndPos(),
		ctx.scopeTree.locations[scopeId].end
	};

	if (cursor.GoToFirstNamedChild())
	{
		++depth;
		
		AddParameterSymbols(cursor, ctx, scopeId, paramVis);

		cursor.GoToParent();
		--depth;
	}

	const size_t symIdx = ctx.symbols.PushBack(std::move(sym));

	if (addedAssignStmt)
	{
		ctx.assignments.symbolIds[assignStmtIdx] = symIdx;
	}
	
	if (cursor.GoToNextNamedSibling())
	{
		assert(cursor.CurrentNode().GetType() == NodeName::kBlock);

		return false; // not handled - go into block in main loop after exit
	}
	else
	{
		cursor.GoToParent(depth);

		return true; // handled
	}
}

void AddGlobalSymbolsForFunctionCall(TreeCursor& cursor, LuaSourceDocParseContext& ctx)
{
	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kFunctionCall);

	const auto fnCallLoc = node.GetSourceLocation();

	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(node, "Failed to go to first child of function_call node");

		return;
	}

	size_t depth = 1;

	node = cursor.CurrentNode();
	if (node.GetType() == NodeName::kIdentifier)
	{
		const auto name = node.GetText();
		if (name.empty())
		{
			ctx.AddError(node, "Function call identifier text was empty");

			cursor.GoToParent(depth);
			return;
		}

		SymbolID prevSymId = GetExistingSymbolID(name,
			node.GetSourceLocation(), SymbolSuperType::Function, ctx);

		if (prevSymId == Null<SymbolID>())
		{
			ctx.AddError(node, std::format("No previous symbol found "
				"for function call '{}'", name));

			cursor.GoToParent(depth);
			return;
		}

		assert(prevSymId < ctx.symbols.Size());
		const auto scopeId = ctx.symbols.GetView<&Symbol::scopeId>(prevSymId);
		assert(scopeId < ctx.scopeTree.Size());

		auto& scopeFlags = ctx.scopeTree.flags[scopeId];
		if ((scopeFlags & ScopeTree::Flag::FunctionInvoked) == 0)
		{
			const auto& symRange = ctx.scopeTree.symbolRanges[scopeId];
			const auto symStart = symRange.min;
			const auto symEnd = std::min(symRange.max, ctx.symbols.Size() - 1);

			std::unordered_set<std::string_view> resolvedSymbols;
			for (size_t i = symEnd; i >= symStart; --i)
			{
				const auto [symName, symClass, symSuper] =
					ctx.symbols.GetView<&Symbol::name, &Symbol::scopeClass,
										&Symbol::super>(i);

				if (symClass != ScopeClass::ScopedGlobal)
				{
					continue;
				}

				ctx.symbols.PushBack({
					.name = symName,
					.location = fnCallLoc,
					.visibility = { fnCallLoc.end, ctx.scopeTree.GetDocumentEnd() },
					.super = symSuper,
					.scopeClass = ScopeClass::UnscopedGlobal,
					.scopeId = ctx.scopeTree.current
				});
			}

			scopeFlags |= ScopeTree::Flag::FunctionInvoked;
		}
		else if (node.GetType() == NodeName::kMethodIndexExpression)
		{
			// user type or table member function
		}
		else if (node.GetType() == NodeName::kDotIndexExpression)
		{
			// table member function
		}
	}
}

void ResolveHintedSymbolTypes(LuaSourceDocParseContext& ctx, 
							  ParsedLuaUserTypeInfo& userTypes)
{
	for (size_t i = 0; i < ctx.declarations.Size(); ++i)
	{
		const auto symId = ctx.declarations.symbolIds[i];
		if (symId >= ctx.symbols.Size())
		{
			continue;
		}

		auto& symType = ctx.symbols.GetView<&Symbol::type>(symId);
		if (symType != kUnresolvedLuaTypeId)
		{
			continue;
		}

		const auto typeText = ctx.declarations.typeText[i];
		if (typeText.empty())
		{
			continue;
		}

		auto userDataIdx = userTypes.GetDataIndex(typeText);
		if (!userDataIdx.IsValid())
		{
			continue;
		}

		symType = userTypes.GetUserTypeId(userDataIdx);
	}
}

void ResolveNestedIndexTypes(LuaSourceDocParseContext& ctx,
							 ParsedLuaUserTypeInfo& userTypes)
{
	for (size_t i = 0; i < ctx.assignments.Size(); ++i)
	{
		const auto symId = ctx.assignments.symbolIds[i];
		if (symId >= ctx.symbols.Size())
		{
			continue;
		}

		auto& symType = ctx.symbols.GetView<&Symbol::type>(symId);
		if (symType != kUnresolvedLuaTypeId)
		{
			continue;
		}

		const auto symLoc = ctx.symbols.GetView<&Symbol::location>(symId);

		const auto& idents = ctx.assignments.identifiers[i];
		const auto& exprTypes = ctx.assignments.indexExprTypes[i];

		if (idents.empty())
		{
			continue;
		}

		const bool maybeEnum = idents.size() == 2 && exprTypes.size() == 1 &&
							   exprTypes[0] == IndexExpressionType::Dot;
		if (maybeEnum)
		{
			// look for ident directly inside userTypes
			auto userDataIdx = userTypes.GetDataIndex(idents[0]);
			if (userDataIdx.IsValid())
			{
				symType = userTypes.GetUserTypeId(userDataIdx);
				continue;
			}
		}

		// look for prev symbol matching ident that is visible
		const auto prevSymId = GetExistingSymbolID(idents[0], symLoc, ctx);
		if (prevSymId >= ctx.symbols.Size())
		{
			continue;
		}

		symType = ctx.symbols.GetView<&Symbol::type>(prevSymId);

		auto userDataIdx = userTypes.GetDataIndex(symType);
		if (!userDataIdx.IsValid())
		{
			continue;
		}

		const auto& fieldNames = userTypes.GetUserTypeFieldNames(userDataIdx);
		const auto& fieldTypes = userTypes.GetUserTypeFieldResultTypeIds(userDataIdx);
		const auto& fieldKinds = userTypes.GetUserTypeFieldMemberKinds(userDataIdx);
		
		for (size_t identIdx = 1; identIdx < idents.size(); ++i)
		{
			if (symType == kInvalidLuaTypeId)
			{
				break;
			}

			assert(identIdx - 1 < exprTypes.size());

			const auto exprType = exprTypes[identIdx - 1];
			auto ident = idents[identIdx];

			for (size_t fieldIdx = 0; fieldIdx < fieldNames.size(); ++fieldIdx)
			{
				if (ident != fieldNames[fieldIdx])
				{
					continue;
				}

				if (MemberKindMatchesIndexExprType(fieldKinds[fieldIdx], exprType))
				{
					symType = fieldTypes[fieldIdx];
				}
				else
				{
					symType = kInvalidLuaTypeId;
				}

				break;
			}
		}
	}
}

} // unnamed

ScopeTree ScopeTree::Parse(TreeCursor& cursor)
{
	ScopeTree tree{};

	//ParseScopeTreeImpl(cursor, tree, false);

	tree.current = 0;

	return tree;
}

bool ScopeTree::SymInsideScope(SourceLocation symLoc, ScopeID scopeId) const
{
	return scopeId < Size() && symLoc.Inside(locations[scopeId]);
}

ScopeID ScopeTree::FindEnclosingFunctionScope(ScopeID scopeId) const
{
	while (scopeId < Size())
	{
		if (kinds[scopeId] == ScopeKind::Function)
		{
			break;
		}
		
		scopeId = scopes[scopeId].parent;
	}

	return scopeId;
}

size_t ScopeTree::GetCurrentScopeEnd() const
{
	return (current < Size()) ? locations[current].end : Null<size_t>();
}

size_t ScopeTree::Size() const
{
	const size_t sz = kinds.size();

	assert(sz == locations.size());
	assert(sz == scopes.size());
	assert(sz == flags.size());

	return sz;
}

size_t ScopeTree::GetDocumentEnd() const
{
	return (locations.size() > 0) ? locations[0].end : Null<size_t>();
}

void LuaSourceDocParseContext::AddError(const TreeNode& node, std::string_view msg)
{
	errors.emplace_back(std::string{ msg }, node.GetSourceLocation());
}

void LuaSourceDocParseContext::AddError(const TreeCursor& cursor, std::string_view msg)
{
	AddError(cursor.CurrentNode(), msg);
}

size_t VariableDeclarations::AddEntry()
{
	const size_t idx = symbolIds.size();

	symbolIds.emplace_back(Null<SymbolID>());
	typeText.emplace_back();
	typeSpans.emplace_back();
	rhsCursors.emplace_back();

	return idx;
}

size_t VariableDeclarations::Size() const
{
	const size_t sz = symbolIds.size();

	assert(sz == typeText.size());
	assert(sz == typeSpans.size());
	assert(sz == rhsCursors.size());

	return sz;
}

size_t AssignmentStatements::AddEntry()
{
	const size_t idx = symbolIds.size();

	symbolIds.emplace_back(Null<SymbolID>());
	identifiers.emplace_back();
	indexExprTypes.emplace_back();
	rhsCursors.emplace_back();

	return idx;
}

void AssignmentStatements::PopEntry()
{
	if (Size() <= 0)
	{
		return;
	}

	symbolIds.pop_back();
	identifiers.pop_back();
	indexExprTypes.pop_back();
	rhsCursors.pop_back();
}

size_t AssignmentStatements::Size() const
{
	const size_t sz = symbolIds.size();

	assert(sz == identifiers.size());
	assert(sz == indexExprTypes.size());
	assert(sz == rhsCursors.size());

	return sz;
}

void ParseLuaSourceDocImpl(TreeCursor& cursor, LuaSourceDocParseContext& ctx)
{
	do
	{
		auto node = cursor.CurrentNode();

		while (NodeEndsCurrentScope(node, ctx.scopeTree))
		{
			PopScope(ctx.scopeTree);
		}

		if (auto kind = GetScopeKind(node))
		{
			PushScope(cursor, ctx, *kind);
		}

		bool handled = false;
		if (node.GetType() == NodeName::kVariableDeclaration)
		{
			ParseVariableDeclaration(cursor, ctx);
			handled = true;
		}
		else if (node.GetType() == NodeName::kAssignmentStatement)
		{
			ParseAssignmentStatement(cursor, ctx);
			handled = true;
		}
		else if (node.GetType() == NodeName::kFunctionDeclaration ||
				 node.GetType() == NodeName::kFunctionDefinition)
		{
			handled = ParseFunction(cursor, ctx);
		}
		else if (node.GetType() == NodeName::kFunctionCall)
		{

		}

		if (!handled && cursor.GoToFirstChild())
		{
			ParseLuaSourceDocImpl(cursor, ctx);

			cursor.GoToParent();
		}

	} while (cursor.GoToNextSibling());
}

void ParseLuaSourceDoc(TreeCursor& cursor, LuaSourceDocParseContext& ctx)
{
	PushScope(cursor, ctx, ScopeKind::Chunk);
	ctx.scopeTree.symbolRanges[ctx.scopeTree.current].min = 0;

	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(cursor, "Failed to go to first child of root node");

		return;
	}

	ParseLuaSourceDocImpl(cursor, ctx);
}

} // ui

#endif