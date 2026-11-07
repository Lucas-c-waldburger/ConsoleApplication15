#include "LuaSourceDocumentSymbol.h"

#if IMGUI_ENABLED
#include <ranges>
#include <ctre/ctre.hpp>
#include "LuaSourceDocumentParser.h"
#include "CursorDive.h"

namespace ui {

namespace {

static constexpr std::string_view kChunkScopeName = "__chunk";
static constexpr std::string_view kBlockScopeName = "__block";


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
		   node.GetSourceLocation().start >= scopeTree.GetCurrentScopeLocation().end;
}

bool IsScopedSymbolKind(LuaSourceDocumentSymbol::Kind kind) noexcept
{
	return kind == LuaSourceDocumentSymbol::Kind::Local ||
		   kind == LuaSourceDocumentSymbol::Kind::Parameter;
}

constexpr bool MemberKindMatchesIndexExprType(LuaUsertypeMemberKind kind,
											  IndexExpressionType type) noexcept
{
	return (type == IndexExpressionType::Dot && IsMemberKindField(kind)) ||
		   (type == IndexExpressionType::Method && IsMemberKindFunction(kind));
}

enum class LuaSourceDocumentScopeType : uint8_t
{
	Invalid,
	Function,
	Block,
	Chunk
};

//LuaSourceDocumentScopeType GetCurrentScopeType(LuaSourceDocumentScopeTree& scopeTree)
//{
//	using ScopeType = LuaSourceDocumentScopeType;
//
//	if (!scopeTree.CurrentValid())
//	{
//		return ScopeType::Invalid;
//	}
//
//
//}

bool InsideFunctionScope(LuaSourceDocumentScopeTree& scopeTree)
{
	if (!scopeTree.CurrentValid())
	{
		return false;
	}

	const size_t origScopeIdx = scopeTree.current;
	bool found = false;

	do
	{
		auto scopeName = scopeTree.GetCurrentScopeName();
		if (!scopeName.empty() && scopeName != kBlockScopeName)
		{
			found = true; 
			break;
		}
	} while (scopeTree.GoToParent());

	scopeTree.SetCurrent(origScopeIdx);

	return found;
}

std::string_view ExtractFunctionCallText(TreeCursor& cursor)
{
	assert(cursor.CurrentNode().GetType() == NodeName::kFunctionCall);

	if (!cursor.GoToFirstChild())
	{
		return {};
	}

	std::string_view result;
	do
	{
		auto node = cursor.CurrentNode();
		if (node.GetType() == NodeName::kIdentifier ||
			node.GetType() == NodeName::kDotIndexExpression ||
			node.GetType() == NodeName::kMethodIndexExpression)
		{
			result = node.GetText();
			break;
		}

	} while (cursor.GoToNextNamedSibling());

	cursor.GoToParent();

	return result;
}

std::string_view GetScopeName(TreeCursor& cursor)
{
	if (cursor.CurrentNode().GetType() != NodeName::kFunctionDeclaration)
	{
		return kBlockScopeName;
	}

	if (!cursor.GoToFirstNamedChild())
	{
		return {};
	}

	std::string_view result;
	do
	{
		auto node = cursor.CurrentNode();

		if (node.GetType() == NodeName::kIdentifier || 
			node.GetType() == NodeName::kDotIndexExpression ||
			node.GetType() == NodeName::kMethodIndexExpression)
		{
			result = node.GetText();
			break;
		}
		
	} while (cursor.GoToNextNamedSibling());

	cursor.GoToParent();

	return result;
}

size_t GetCurrentScopeTableFunctionIndex(const LuaSourceDocumentSymbolParseContext& ctx)
{
	if (!ctx.scopeTree.CurrentValid())
	{
		return std::numeric_limits<size_t>::max();
	}

	auto scopeName = ctx.scopeTree.GetCurrentScopeName();
	if (const auto sep = scopeName.find_last_of(".:"); sep != std::string_view::npos)
	{
		scopeName.remove_prefix(sep + 1);
	}

	for (size_t i = 0; i < ctx.signaturesTable.functionNames.size(); ++i)
	{
		if (ctx.signaturesTable.functionNames[i] == scopeName)
		{
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GetFunctionCallScopeIndex(TreeCursor& cursor, 
								 const LuaSourceDocumentScopeTree& scopeTree)
{
	if (!scopeTree.CurrentValid())
	{
		return std::numeric_limits<size_t>::max();
	}

	const auto fnCallText = ExtractFunctionCallText(cursor);
	if (fnCallText.empty())
	{
		return std::numeric_limits<size_t>::max();
	}

	const size_t origScopeIdx = scopeTree.current;

	for (size_t i = 0; i < scopeTree.names.size(); ++i)
	{
		if (fnCallText == scopeTree.names[i])
		{
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

void AddGlobalSymbolsForFunctionCall(TreeCursor& cursor,
									 LuaSourceDocumentSymbolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	const size_t fnScopeIdx = GetFunctionCallScopeIndex(cursor, ctx.scopeTree);
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

		auto nodeLoc = cursor.CurrentNode().GetSourceLocation();

		ctx.symbols.PushBack(Sym{
			.name = symName,
			.location = nodeLoc,
			.scope = { nodeLoc.start, ctx.scopeTree.locations[0].end },
			.type = symType,
			.kind = Sym::Kind::UnscopedGlobal
		});
	}
}

void TraverseScopeTree(const SourceLocation& symLoc,
					   LuaSourceDocumentScopeTree& scopeTree)
{
	assert(scopeTree.CurrentValid());

	if (scopeTree.AtLeaf())
	{
		return;
	}

	for (const size_t child : scopeTree.nodes[scopeTree.current].children)
	{
		if (symLoc.Inside(scopeTree.locations[child]))
		{
			scopeTree.SetCurrent(child);
			
			return TraverseScopeTree(symLoc, scopeTree);
		}
	}

	return;
}

SourceLocation
GetScopeForSymLocation(const SourceLocation& symLoc,
					   LuaSourceDocumentScopeTree& scopeTree)
{
	const size_t origScopeIdx = scopeTree.current;

	scopeTree.Reset();

	TraverseScopeTree(symLoc, scopeTree);

	SourceLocation result{};
	if (symLoc.Inside(scopeTree.GetCurrentScopeLocation()))
	{
		result = scopeTree.GetCurrentScopeLocation();
	}

	scopeTree.SetCurrent(origScopeIdx);

	return result;
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

// Go through previously added symbols and find the one with same name & scope
size_t GetSymIndexForFoundIdentifier(std::string_view ident,
									 const SourceLocation& newSymLoc,
									 LuaSourceDocumentSymbolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	for (size_t i = ctx.symbols.Size(); i-- > 0;)
	{
		const auto [symName, symScope] =
			ctx.symbols.GetView<&Sym::name, &Sym::scope>(i);

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

uint32_t GetSymTypeForIndexExpressionNode(const TreeNode& node, 
										  const SourceLocation& newSymLoc,
										  LuaSourceDocumentSymbolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	auto exprInfo = ExtractIndexExpressionInfo(node);
	if (!exprInfo)
	{
		return kInvalidLuaTypeId;
	}

	if (exprInfo->type == IndexExpressionType::Dot)
	{
		// try to get type immediately from userTypeInfo if lhs is actually just an enum
		auto dataIdx = ctx.userTypeInfo.GetDataIndex(exprInfo->lhsText);
		if (dataIdx.IsValid())
		{
			return ctx.userTypeInfo.GetUserTypeId(dataIdx);
		}
	}

	// not an enum - try to get type of lhs object from prev parsed symbols in current scope
	size_t symIndex = GetSymIndexForFoundIdentifier(exprInfo->lhsText, newSymLoc, ctx);
	if (symIndex >= ctx.symbols.Size())
	{
		return kInvalidLuaTypeId;
	}

	const auto symType = ctx.symbols.GetView<&Sym::type>(symIndex);

	const auto dataIdx = ctx.userTypeInfo.GetDataIndex(symType);
	if (!dataIdx.IsValid())
	{
		return kInvalidLuaTypeId;
	}

	const auto& fieldNames = ctx.userTypeInfo.GetUserTypeFieldNames(dataIdx);
	const auto& fieldKinds = ctx.userTypeInfo.GetUserTypeFieldMemberKinds(dataIdx);
	const auto& fieldResultTypes = ctx.userTypeInfo.GetUserTypeFieldResultTypeIds(dataIdx);

	// for lhs object type, find its member being referenced by rhs and either
	// get its return type (if method) or member type (if field)
	for (size_t i = 0; i < fieldNames.size(); ++i)
	{
		if (fieldNames[i] != exprInfo->rhsText)
		{
			continue;
		}

		if (MemberKindMatchesIndexExprType(fieldKinds[i], exprInfo->type))
		{
			return fieldResultTypes[i];
		}
		else
		{
			return kInvalidLuaTypeId;
		}
	}

	return kInvalidLuaTypeId;
}

uint32_t GetSymTypeForRhsExpression(TreeCursor& cursor, SourceLocation symLoc,
									LuaSourceDocumentSymbolParseContext& ctx)
{
	auto node = cursor.CurrentNode();
	assert(node);

	if (node.GetType() == NodeName::kNumber)
	{
		return kNativeNumberLuaTypeId;
	}
	else if (node.GetType() == NodeName::kBoolean)
	{
		return kNativeBooleanLuaTypeId;
	}
	else if (node.GetType() == NodeName::kString)
	{
		return kNativeStringLuaTypeId;
	}
	else if (node.GetType() == NodeName::kTableConstructor)
	{
		return kNativeTableLuaTypeId;
	}
	else if (node.GetType() == NodeName::kIdentifier)
	{
		auto ident = node.GetText();

		const size_t symIdx = GetSymIndexForFoundIdentifier(ident, symLoc, ctx);

		return (symIdx < ctx.symbols.Size()) 
			? ctx.symbols.GetView<&LuaSourceDocumentSymbol::type>(symIdx) 
			: kInvalidLuaTypeId;
	}
	else if (IsIndexExpressionNode(node))
	{
		return GetSymTypeForIndexExpressionNode(node, symLoc, ctx);
	}
	else if (node.GetType() == NodeName::kFunctionCall)
	{
		// user-defined function declared in this document
		const size_t fnScope = GetFunctionCallScopeIndex(cursor, ctx.scopeTree);
		if (fnScope < ctx.scopeTree.Size())
		{
			return ctx.scopeTree.returns[fnScope];
		}

		// method/field call on a typed object (e.g. e:getTransform())
		uint32_t symType = kInvalidLuaTypeId;
		if (cursor.GoToFirstNamedChild())
		{
			const auto callee = cursor.CurrentNode();
			if (IsIndexExpressionNode(callee))
			{
				symType = GetSymTypeForIndexExpressionNode(callee, symLoc, ctx);
			}

			cursor.GoToParent();
		}

		return symType;
	}

	return kInvalidLuaTypeId;
}

struct AssignmentStatementParts
{
	TreeNode lhs; // identifier being assigned
	TreeCursor rhs; // positioned on the rhs expression when HasRhs()
	SourceLocation stmtLocation; // whole assignment_statement
	bool operator!() const { return !lhs || lhs.GetText().empty(); }
	bool HasRhs() const { return rhs; }
};

// cursor must be on an assignment_statement. Does not move the passed cursor.
// Only single `identifier = expr` assignments are handled (not `p.x = ...`).
AssignmentStatementParts GetAssignmentStatementParts(TreeCursor& cursor)
{
	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kAssignmentStatement);

	if (node.GetNamedChildCount() != 2)
	{
		return {};
	}

	AssignmentStatementParts parts{
		.stmtLocation = node.GetSourceLocation()
	};

	if (!cursor.GoToFirstNamedChild())
	{
		return {};
	}

	node = cursor.CurrentNode();
	if (node.GetType() != NodeName::kVariableList ||
		node.GetNamedChildCount() != 1)
	{
		cursor.GoToParent();
		return {};
	}

	if (!cursor.GoToFirstNamedChild())
	{
		cursor.GoToParent();
		return {};
	}

	node = cursor.CurrentNode();
	if (node.GetType() != NodeName::kIdentifier)
	{
		cursor.GoToParent(2);
		return {};
	}

	parts.lhs = node;

	cursor.GoToParent(); // back to var list

	if (GoToSiblingWithNodeNameAndNamedChildCount(
		cursor, NodeName::kExpressionList, 1))
	{
		cursor.GoToParent();
		return parts;
	}

	if (!cursor.GoToFirstNamedChild())
	{
		cursor.GoToParent();
		return parts;
	}

	parts.rhs = cursor.Branch();

	cursor.GoToParent(2);

	return parts;
}

LuaSourceDocumentSymbol::Kind 
GetSymKindForNewDeclarationAndAdvance(TreeCursor& cursor, 
	LuaSourceDocumentSymbolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	if (cursor.CurrentNode().GetText() == kLocalName)
	{
		cursor.GoToNextNamedSibling();
		return Sym::Kind::Local;
	}

	return (InsideFunctionScope(ctx.scopeTree))
		? Sym::Kind::ScopedGlobal
		: Sym::Kind::UnscopedGlobal;
}

LuaSourceDocumentSymbol::Kind 
GetSymKindForAssignment(std::string_view ident, SourceLocation loc,
						LuaSourceDocumentSymbolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	const size_t prev = GetSymIndexForFoundIdentifier(ident, loc, ctx);
	if (prev < ctx.symbols.Size())
	{
		return ctx.symbols.GetView<&Sym::kind>(prev);
	}

	return (InsideFunctionScope(ctx.scopeTree)) 
		? Sym::Kind::ScopedGlobal
		: Sym::Kind::UnscopedGlobal;
}

void AddSymForAssignmentStatement(TreeCursor& cursor,
	LuaSourceDocumentSymbolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	assert(cursor.CurrentNode().GetType() == NodeName::kAssignmentStatement);
	assert(ctx.scopeTree.CurrentValid());

	auto parts = GetAssignmentStatementParts(cursor);
	if (!parts)
	{
		return;
	}

	const auto lhsText = parts.lhs.GetText();
	const auto lhsLoc = parts.lhs.GetSourceLocation();

	// look for previous symbol with this name that is visible here to determine kind
	const auto kind = GetSymKindForAssignment(lhsText, lhsLoc, ctx);

	// symbol is live from the end of this statement to the end of the innermost 
	// scope containing it (or the end of the root chunk for unscoped globals)
	const size_t scopeEnd = (kind == Sym::Kind::UnscopedGlobal)
		? ctx.scopeTree.GetDocumentEnd()
		: ctx.scopeTree.GetCurrentScopeLocation().end;

	ctx.symbols.PushBack(Sym{
		.name = lhsText,
		.location = lhsLoc,
		.scope = { parts.stmtLocation.end, scopeEnd },
		.type = parts.HasRhs() ? GetSymTypeForRhsExpression(parts.rhs, lhsLoc, ctx)
							   : kInvalidLuaTypeId,
		.kind = kind
	});

	if (parts.HasRhs() && parts.rhs.CurrentNode().GetType() == NodeName::kFunctionCall)
	{
		AddGlobalSymbolsForFunctionCall(parts.rhs, ctx);
	}
}

void AddSymForVariableDeclaration(TreeCursor& cursor,
	LuaSourceDocumentSymbolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	assert(cursor.CurrentNode().GetType() == NodeName::kVariableDeclaration);
	assert(ctx.scopeTree.CurrentValid());

	if (!cursor.GoToFirstChild())
	{
		return;
	}

	const Sym::Kind kind = GetSymKindForNewDeclarationAndAdvance(cursor, ctx);

	if (cursor.CurrentNode().GetType() != NodeName::kAssignmentStatement)
	{
		cursor.GoToParent();
		return;
	}

	auto parts = GetAssignmentStatementParts(cursor);
	if (!parts)
	{
		cursor.GoToParent();
		return;
	}

	const auto lhsLoc = parts.lhs.GetSourceLocation();

	ctx.symbols.PushBack(Sym{
		.name = parts.lhs.GetText(),
		.location = lhsLoc,
		.scope = { parts.stmtLocation.end, ctx.scopeTree.GetCurrentScopeLocation().end },
		.type = parts.HasRhs() ? GetSymTypeForRhsExpression(parts.rhs, lhsLoc, ctx)
							   : kInvalidLuaTypeId,
		.kind = kind
	});

	if (parts.HasRhs() && parts.rhs.CurrentNode().GetType() == NodeName::kFunctionCall)
	{
		AddGlobalSymbolsForFunctionCall(parts.rhs, ctx);
	}

	cursor.GoToParent();
}

//void AddSymForVariableDeclaration(TreeCursor& cursor, 
//                                  LuaSourceDocumentSymbolParseContext& ctx)
//{
//    using Sym = LuaSourceDocumentSymbol;
//
//    assert(cursor.CurrentNode().GetType() == NodeName::kVariableDeclaration);
//    assert(ctx.scopeTree.CurrentValid());
//
//    auto c = cursor.Branch();
//    if (!c.GoToFirstNamedChild()) // skips the anonymous `local` token
//    {
//        return;
//    }
//
//    SingleAssignment a;
//    if (!ReadSingleAssignment(c, a) || a.lhs.GetText().empty())
//    {
//        return;
//    }
//
//    const auto lhsLoc = a.lhs.GetSourceLocation();
//
//    ctx.symbols.PushBack(Sym{
//        .name = a.lhs.GetText(),
//        .location = lhsLoc,
//        .scope = { a.statement.end, ctx.scopeTree.GetCurrentScopeLocation().end },
//        .type = a.hasRhs ? GetSymTypeForRhsExpression(a.rhs, lhsLoc, ctx) 
//                         : kInvalidLuaTypeId,
//        .kind = Sym::Kind::Local
//    });
//
//    if (a.hasRhs && a.rhs.CurrentNode().GetType() == NodeName::kFunctionCall)
//    {
//        AddGlobalSymbolsForFunctionCall(a.rhs, ctx);
//    }
//}

bool ParseLuaSourceDocumentTableInfo(TreeCursor& cursor, 
									 LuaSourceDocumentSymbolParseContext& ctx)
{
	if (!cursor.GoToFirstChild())
	{
		ctx.AddError(cursor.CurrentNode(), 
			"Failed to go to first child of root cursor");

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
					ctx.AddError(node, 
						"Expected 2 named children in metatable args node");

					return false;
				}

				if (!cursor.GoToFirstNamedChild())
				{
					ctx.AddError(node, 
						"Failed to go to first named child of metatable args node");

					return false;
				}

				node = cursor.CurrentNode();
				if (node.GetType() != NodeName::kIdentifier)
				{
					ctx.AddError(node, 
						"Expected first named child of metatable args "
						"node to be an identifier");

					return false;
				}

				auto fnTableText = node.GetText();
				if (fnTableText.empty())
				{
					ctx.AddError(node, 
						"Function table name from identifier node was empty");

					return false;
				}

				ctx.tableInfo.functionTableName = fnTableText;

				if (!cursor.GoToNextNamedSibling())
				{
					ctx.AddError(node, 
						"Failed to go to second named child of metatable args node");

					return false;
				}

				node = cursor.CurrentNode();
				if (node.GetType() != NodeName::kIdentifier)
				{
					ctx.AddError(node, 
						"Expected second named child of metatable "
						"args node to be an identifier");

					return false;
				}

				auto metaTableText = node.GetText();
				if (metaTableText.empty())
				{
					ctx.AddError(node, 
						"Metatable name from identifier node was empty");

					return false;
				}

				ctx.tableInfo.metaTableName = metaTableText;

				return true;
			}

		} while (cursor.GoToNextSibling());

		cursor.GoToParent();

	} while (cursor.GoToNextSibling());

	ctx.AddError(cursor.CurrentNode(), 
		"Failed to parse metatable function call and arguments");

	return false;
}



void AddFunctionParameterSymbols(TreeCursor& cursor, 
							   LuaSourceDocumentSymbolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	assert(cursor.CurrentNode().GetType() == NodeName::kParameters);

	if (!cursor.GoToFirstChild())
	{
		return;
	}

	size_t tableFnIndex = GetCurrentScopeTableFunctionIndex(ctx);
	const bool insideTableFn = tableFnIndex < ctx.signaturesTable.functionNames.size();

	size_t paramIndex = 0;
	do
	{
		auto node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kIdentifier)
		{
			continue;
		}

		auto paramName = node.GetText();
		if (paramName.empty())
		{
			continue;
		}

		assert(ctx.scopeTree.CurrentValid());

		auto nodeLoc = node.GetSourceLocation();

		Sym sym{
			.name = paramName,
			.location = nodeLoc,
			.scope = { 
				nodeLoc.start, 
				ctx.scopeTree.GetCurrentScopeLocation().end 
			},
			.type = kInvalidLuaTypeId,
			.kind = Sym::Kind::Parameter
		};

		if (insideTableFn)
		{
			// add param type to symbol, and assign param name to sigs
			auto& sigs = ctx.signaturesTable;

			assert(sigs.parameterNames.size() == sigs.parameterTypes.size());
			assert(tableFnIndex < sigs.parameterNames.size());
			
			auto& paramNames = sigs.parameterNames[tableFnIndex];
			const auto& paramTypes = sigs.parameterTypes[tableFnIndex];

			assert(paramNames.size() == paramTypes.size());
			assert(paramIndex < paramNames.size());

			paramNames[paramIndex] = paramName;
			sym.type = paramTypes[paramIndex];
		}

		ctx.symbols.PushBack(std::move(sym));

		++paramIndex;

	} while (cursor.GoToNextNamedSibling());

	cursor.GoToParent();
}

void AddScopeReturnType(TreeCursor& cursor, 
					    LuaSourceDocumentSymbolParseContext& ctx)
{
	assert(cursor.CurrentNode().GetType() == NodeName::kReturnStatement);

	// if inside inner fn block, get the function scope that it's inside
	const size_t parentFnScopeIdx = GetCurrentParentFunctionScopeIndex(ctx.scopeTree);
	if (parentFnScopeIdx >= ctx.scopeTree.Size())
	{
		return;
	}

	if (cursor.CurrentNode().GetNamedChildCount() != 1)
	{
		return;
	}
	if (!cursor.GoToFirstNamedChild())
	{
		return;
	}

	// return_statement -> expression_list -> expression
	if (cursor.CurrentNode().GetType() == NodeName::kExpressionList &&
		cursor.CurrentNode().GetNamedChildCount() == 1 &&
		cursor.GoToFirstNamedChild())
	{
		auto exprNode = cursor.CurrentNode();

		const uint32_t retType = GetSymTypeForRhsExpression(
			cursor, exprNode.GetSourceLocation(), ctx);

		if (exprNode.GetType() == NodeName::kFunctionCall)
		{
			AddGlobalSymbolsForFunctionCall(cursor, ctx);
		}

		ctx.scopeTree.returns[parentFnScopeIdx] = retType;

		cursor.GoToParent();
	}

	cursor.GoToParent();
}

bool ParseLuaSourceDocumentSymbolsImpl(TreeCursor& cursor, 
									   LuaSourceDocumentSymbolParseContext& ctx)
{
	if (!cursor.GoToFirstChild())
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
		else if (node.GetType() == NodeName::kVariableDeclaration)
		{
			AddSymForVariableDeclaration(cursor, ctx);
			handled = true;
		}
		else if (node.GetType() == NodeName::kFunctionCall)
		{
			AddGlobalSymbolsForFunctionCall(cursor, ctx);
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

	cursor.GoToParent();

	return true;
}

size_t GetSymbolSearchStartIndex(size_t byteOffset,
								 LuaSourceDocumentSymbolSOA& symbols)
{
	using Sym = LuaSourceDocumentSymbol;

	size_t low = 0;
	size_t high = symbols.Size();

	size_t searchStart = std::numeric_limits<size_t>::max();

	if (high == 0)
	{
		return std::numeric_limits<size_t>::max();
	}

	while (low < high) 
	{
		std::size_t mid = low + (high - low) / 2;

		auto loc = symbols.GetView<&Sym::location>(mid);
		if (loc.start >= byteOffset)
		{
			high = mid;
		}
		else
		{
			low = mid + 1;
		}
	}

	if (low > 0)
	{
		searchStart = low - 1;
	}

	return searchStart;
}

size_t GetSymbolSearchStartIndex(SourceLocation loc,
								 LuaSourceDocumentSymbolSOA& symbols)
{
	return GetSymbolSearchStartIndex(loc.start, symbols);
}

} // unnamed

bool LuaSourceDocumentScopeTree::CurrentValid() const
{
	assert(locations.size() == names.size());
	assert(locations.size() == nodes.size());

	return current < nodes.size() && nodes[current].index == current;
}

bool LuaSourceDocumentScopeTree::SetCurrent(size_t idx)
{
	assert(locations.size() == names.size());
	assert(locations.size() == nodes.size());

	if (idx >= nodes.size())
	{
		return false;
	}

	current = idx;
	return true;
}

size_t LuaSourceDocumentScopeTree::Size() const noexcept
{
	assert(locations.size() == names.size());
	assert(locations.size() == nodes.size());

	return nodes.size();
}

bool LuaSourceDocumentScopeTree::GoToParent()
{
	assert(locations.size() == names.size());
	assert(locations.size() == nodes.size());

	if (current < nodes.size() && nodes[current].parent < nodes.size())
	{
		current = nodes[current].parent;
		return true;
	}
	return false;
}

bool LuaSourceDocumentScopeTree::PushScope(TreeCursor& cursor)
{
	assert(locations.size() == names.size());
	assert(locations.size() == nodes.size());

	const bool isRoot = nodes.empty();
	if (!isRoot && current >= nodes.size())
	{
		return false;
	}

	auto treeNode = cursor.CurrentNode();
	assert(treeNode);

	const size_t newIdx = nodes.size();

	if (!isRoot)
	{
		nodes[current].children.emplace_back(newIdx);
	}
	locations.emplace_back(treeNode.GetSourceLocation());
	names.emplace_back(GetScopeName(cursor));
	returns.emplace_back(kInvalidLuaTypeId);

	auto& newScope = nodes.emplace_back();
	newScope.index = newIdx;
	newScope.parent = isRoot 
		? std::numeric_limits<size_t>::max() 
		: current;

	current = newIdx;
	return true;
}

std::string_view LuaSourceDocumentScopeTree::GetCurrentScopeName() const
{
	if (!CurrentValid())
	{
		return {};
	}

	return names[current];
}

size_t LuaSourceDocumentScopeTree::GetDocumentEnd() const
{
	if (locations.empty())
	{
		return 0;
	}
	return locations[0].end;
}

SourceLocation LuaSourceDocumentScopeTree::GetCurrentScopeLocation() const
{
	if (!CurrentValid())
	{
		return {};
	}

	return locations[current];
}

void LuaSourceDocumentSymbolParseContext::AddError(const TreeNode& node, std::string_view msg)
{
	errors.emplace_back(std::string{ msg }, node.GetSourceLocation());
}

bool ParseLuaSourceDocumentSymbols(TreeCursor& cursor,
	LuaSourceDocumentSymbolParseContext& ctx)
{
	if (!cursor)
	{
		return false;
	}

	assert(cursor.CurrentNode().GetType() == NodeName::kChunk);

	ctx.scopeTree.PushScope(cursor);

	return ParseLuaSourceDocumentSymbolsImpl(cursor, ctx);
}

size_t FindMatchingSymbolIndex(std::string_view targetName,
							   size_t byteOffset,
							   LuaSourceDocumentSymbolSOA& symbols)
{
	using Sym = LuaSourceDocumentSymbol;

	const size_t searchStart = GetSymbolSearchStartIndex(byteOffset, symbols);
	if (searchStart >= symbols.Size())
	{
		return std::numeric_limits<size_t>::max();
	}

	for (size_t i = searchStart + 1; i-- > 0;)
	{
		auto [symName, symScope] = symbols.GetView<&Sym::name, &Sym::scope>(i);

		if (symName == targetName && symScope.Contains(byteOffset))
		{
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t FindMatchingSymbolIndex(std::string_view targetName,
							   SourceLocation targetLoc,
							   LuaSourceDocumentSymbolSOA& symbols)
{
	using Sym = LuaSourceDocumentSymbol;

	const size_t searchStart = GetSymbolSearchStartIndex(targetLoc, symbols);
	if (searchStart >= symbols.Size())
	{
		return std::numeric_limits<size_t>::max();
	}

	for (size_t i = searchStart + 1; i-- > 0;)
	{
		auto [symName, symScope] = symbols.GetView<&Sym::name, &Sym::scope>(i);

		if (symName == targetName && targetLoc.Inside(symScope))
		{
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

} // ui

#endif
