#include "LuaSourceDocumentSymbol.h"

#if IMGUI_ENABLED
#include <ranges>
#include <ctre/ctre.hpp>
#include "LuaSourceDocumentParser.h"

namespace ui {

namespace {

static constexpr std::string_view kBlockScopeName = "__block";

constexpr std::string_view GetRawTypeName(std::string_view typeNameWithCvRefQualifiers)
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
		   node.GetSourceLocation().start > scopeTree.GetCurrentScopeLocation().end;
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

bool InsideFunctionScope(LuaSourceDocumentScopeTree& scopeTree)
{
	if (!scopeTree.CurrentValid())
	{
		return false;
	}

	using ScopeType = LuaSourceDocumentScopeType;

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

bool IsCurrentScopeTableFunction(LuaSourceDocumentSybolParseContext& ctx)
{
	if (!ctx.scopeTree.CurrentValid())
	{
		return false;
	}

	return std::ranges::contains(ctx.signaturesTable.functionNames, 
								 ctx.scopeTree.GetCurrentScopeName());
}

size_t GetCurrentScopeTableFunctionIndex(const LuaSourceDocumentSybolParseContext& ctx)
{
	if (!ctx.scopeTree.CurrentValid())
	{
		return std::numeric_limits<size_t>::max();
	}

	for (size_t i = 0; i < ctx.signaturesTable.functionNames.size(); ++i)
	{
		if (ctx.signaturesTable.functionNames[i] == 
			ctx.scopeTree.GetCurrentScopeName())
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
									 LuaSourceDocumentSybolParseContext& ctx)
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

		if (symLoc.start > scopeLoc.end)
		{
			break;
		} 
		if (symKind != Sym::Kind::ScopedGlobal)
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

uint32_t GetTableFunctionParamSymType(std::string_view paramName,
									  const LuaSourceDocumentSybolParseContext& ctx)
{
	const auto& sigs = ctx.signaturesTable;

	const auto idx = GetCurrentScopeTableFunctionIndex(ctx);
	if (idx >= sigs.functionNames.size())
	{
		return kInvalidLuaTypeId;
	}

	assert(sigs.functionNames.size() == sigs.parameterNames.size());
	assert(sigs.parameterNames.size() == sigs.parameterTypes.size());

	const auto& paramNames = sigs.parameterNames[idx];
	const auto& paramTypes = sigs.parameterTypes[idx];

	assert(paramNames.size() == paramTypes.size());

	for (size_t i = 0; i < paramNames.size(); ++i)
	{
		if (paramNames[i] == paramName)
		{
			return paramTypes[i];
		}
	}

	return kInvalidLuaTypeId;
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

	if (symLoc.Inside(scopeTree.GetCurrentScopeLocation()))
	{
		scopeTree.SetCurrent(origScopeIdx);

		return scopeTree.GetCurrentScopeLocation();
	}

	scopeTree.SetCurrent(origScopeIdx);

	return {};
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
									 LuaSourceDocumentSybolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	for (size_t i = ctx.symbols.Size() > 0; i-- > 0;)
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
										  LuaSourceDocumentSybolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	auto exprInfo = ExtractIndexExpressionInfo(node);
	if (!exprInfo)
	{
		return kInvalidLuaTypeId;
	}

	// get type of lhs object from prev parsed symbols in current scope
	size_t symIndex = GetSymIndexForFoundIdentifier(exprInfo->lhsText, newSymLoc, 
													ctx);
	if (symIndex >= ctx.symbols.Size())
	{
		return kInvalidLuaTypeId;
	}

	const auto symType = ctx.symbols.GetView<&Sym::type>(symIndex);
	auto dataIdx = ctx.userTypeInfo.GetDataIndex(symType);
	if (!dataIdx.IsValid())
	{
		return kInvalidLuaTypeId;
	}

	const auto& fieldNames = ctx.userTypeInfo.GetUserTypeFieldNames(dataIdx);
	const auto& fieldKinds = ctx.userTypeInfo.GetUserTypeFieldMemberKinds(dataIdx);
	const auto& fieldResultTypes = ctx.userTypeInfo.GetUserTypeFieldResultTypeIds(dataIdx);

	// for lhs object type, find its member being referenced by rhs and get 
	// its return type (if method) or member type (if field)
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
									LuaSourceDocumentSybolParseContext& ctx)
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

		return (symIdx < ctx.symbols.Size()) ? 
			ctx.symbols.GetView<&LuaSourceDocumentSymbol::type>(symIdx) : 
			kInvalidLuaTypeId;
	}
	else if (IsIndexExpressionNode(node))
	{
		return GetSymTypeForIndexExpressionNode(node, symLoc, ctx);
	}
	else if (node.GetType() == NodeName::kFunctionCall)
	{
		if (node.GetChildCount() == 1 && cursor.GoToFirstChild())
		{
			uint32_t symType = kInvalidLuaTypeId;

			node = cursor.CurrentNode();
			if (IsIndexExpressionNode(node))
			{
				symType = GetSymTypeForIndexExpressionNode(node, symLoc, ctx);
			}
			else if (node.GetType() == NodeName::kIdentifier)
			{
				auto ident = node.GetText();
				const size_t symIdx = GetSymIndexForFoundIdentifier(ident, symLoc, ctx);
				symType = (symIdx < ctx.symbols.Size()) ? 
					ctx.symbols.GetView<&LuaSourceDocumentSymbol::type>(symIdx) : 
					kInvalidLuaTypeId;
			}

			cursor.GoToParent();

			return symType;
		}
	}

	return kInvalidLuaTypeId;
}

void AddSymForAssignmentStatement(TreeCursor& cursor,
							  LuaSourceDocumentSybolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kAssignmentStatement);

	if (node.GetNamedChildCount() != 2)
	{
		return;
	}
	if (!cursor.GoToFirstNamedChild())
	{
		return;
	}

	auto varListNode = cursor.CurrentNode();
	if (varListNode.GetType() != NodeName::kVariableList ||
		varListNode.GetChildCount() != 1) // only handle single var decls
	{
		cursor.GoToParent();
		return;
	}

	if (!cursor.GoToFirstNamedChild())
	{
		cursor.GoToParent();
		return;
	}

	auto lhsIdentNode = cursor.CurrentNode();
	if (lhsIdentNode.GetType() != NodeName::kIdentifier)
	{
		// skip inner type idexing assignment - only top level type reassignment
		cursor.GoToParent(2);
		return;
	}

	auto lhsText = lhsIdentNode.GetText(); // what are we reassigning
	if (lhsText.empty())
	{
		cursor.GoToParent(2);
		return;
	}
	
	auto nodeLoc = lhsIdentNode.GetSourceLocation();

	Sym newSym{
		.name = lhsText,
		.location = nodeLoc,
	};

	const size_t symIndex = GetSymIndexForFoundIdentifier(newSym.name, 
														  newSym.location, ctx);
	cursor.GoToParent();

	if (symIndex >= ctx.symbols.Size())
	{
		cursor.GoToParent();
		return;
	}

	const auto [symScope, symKind] = ctx.symbols.GetView<&Sym::scope, 
														 &Sym::kind>(symIndex);
	newSym.scope = {
		nodeLoc.start,
		std::max(nodeLoc.end, symScope.end) // either cur scope or end if unscoped global
	};
	newSym.kind = symKind;

	if (!cursor.GoToNextNamedSibling())
	{
		ctx.symbols.PushBack(std::move(newSym));
		cursor.GoToParent();
		return;
	}

	auto exprListNode = cursor.CurrentNode();
	if (exprListNode.GetType() != NodeName::kExpressionList ||
		exprListNode.GetChildCount() != 1)
	{
		ctx.symbols.PushBack(std::move(newSym));
		cursor.GoToParent();
		return;
	}

	if (!cursor.GoToFirstNamedChild())
	{
		ctx.symbols.PushBack(std::move(newSym));
		cursor.GoToParent();
		return;
	}

	auto rhsExprNode = cursor.CurrentNode();
	newSym.type = GetSymTypeForRhsExpression(cursor, newSym.location, ctx);

	ctx.symbols.PushBack(std::move(newSym));

	if (rhsExprNode.GetType() == NodeName::kFunctionCall)
	{
		AddGlobalSymbolsForFunctionCall(cursor, ctx);
	}

	cursor.GoToParent(2);
}

bool ParseLuaSourceDocumentSignaturesTableImpl(TreeCursor& cursor,
										   LuaSourceDocumentSignaturesTable& sigTable,
										   const ParsedLuaUserTypeInfo& userTypeInfo)
{
	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kTableConstructor);

	if (!cursor.GoToFirstChild())
	{
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
		if (cursor.CurrentNode().GetType() != NodeName::kIdentifier)
		{
			cursor.GoToHomeDepth();
			continue;
		}

		node = cursor.CurrentNode();
		auto fnIdent = node.GetText();
		if (fnIdent.empty())
		{
			cursor.GoToHomeDepth();
			continue;
		}

		bool foundTableCtor = false;
		while (cursor.GoToNextNamedSibling())
		{
			if (cursor.CurrentNode().GetType() == NodeName::kTableConstructor)
			{
				foundTableCtor = true;
				break;
			}
		}
		if (!foundTableCtor)
		{
			cursor.GoToHomeDepth();
			continue;
		}

		sigTable.functionNames.emplace_back(fnIdent);
		sigTable.parameterNames.emplace_back();
		auto& paramTypes = sigTable.parameterTypes.emplace_back();

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
			auto rawParamType = GetRawTypeName(paramType);
			if (!rawParamType.empty())
			{
				if (auto dataIdx = userTypeInfo.GetDataIndex(rawParamType);
					dataIdx.IsValid())
				{
					paramTypes.emplace_back(userTypeInfo.GetUserTypeId(dataIdx));
				}
			}

			paramCursor.GoToHomeDepth();

		} while (paramCursor.GoToNextNamedSibling());

	} while (cursor.GoToNextNamedSibling());

	return true;
}

void AddSymForVariableDeclaration(TreeCursor& cursor, 
								  LuaSourceDocumentSybolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	auto node = cursor.CurrentNode();

	assert(node.GetType() == NodeName::kVariableDeclaration);
	assert(!ctx.scopeTree.CurrentValid());

	if (!cursor.GoToFirstChild())
	{
		return;
	}

	Sym newSym{};

	node = cursor.CurrentNode();
	if (node.GetText() == "local")
	{
		newSym.kind = Sym::Kind::Local;

		if (!cursor.GoToNextSibling())
		{
			cursor.GoToParent();
			return;
		}
	}
	else
	{
		newSym.kind = InsideFunctionScope(ctx.scopeTree) 
			? Sym::Kind::ScopedGlobal
			: Sym::Kind::UnscopedGlobal;
	}

	node = cursor.CurrentNode();
	if (node.GetType() != NodeName::kAssignmentStatement ||
		node.GetNamedChildCount() != 2)
	{
		cursor.GoToParent();
		return;
	}

	if (!cursor.GoToFirstNamedChild())
	{
		cursor.GoToParent();
		return;
	}

	node = cursor.CurrentNode();
	if (node.GetChildCount() != 2)
	{
		cursor.GoToParent(2);
		return;
	}

	if (!cursor.GoToFirstChild())
	{
		cursor.GoToParent(2);
		return;
	}

	auto varListNode = cursor.CurrentNode();
	if (varListNode.GetType() != NodeName::kVariableList ||
		varListNode.GetChildCount() != 1) // only handle single var decls
	{
		cursor.GoToParent(3);
		return;
	}

	if (!cursor.GoToFirstChild())
	{
		cursor.GoToParent(3);
		return;
	}

	auto varIdentNode = cursor.CurrentNode();
	if (varIdentNode.GetType() != NodeName::kIdentifier)
	{
		cursor.GoToParent(4);
		return;
	}

	auto varName = varIdentNode.GetText();
	if (varName.empty())
	{
		cursor.GoToParent(4);
		return;
	}

	auto nodeLoc = varIdentNode.GetSourceLocation();
	auto symScopeEnd = (newSym.kind == LuaSourceDocumentSymbol::Kind::UnscopedGlobal)
		? ctx.scopeTree.GetCurrentScopeLocation().end
		: nodeLoc.end;

	newSym.name = varName;
	newSym.location = nodeLoc;
	newSym.scope = { nodeLoc.start, symScopeEnd };

	cursor.GoToParent(2);

	if (!cursor.GoToNextNamedSibling())
	{
		ctx.symbols.PushBack(std::move(newSym));
		cursor.GoToParent(2);
		return;
	}

	auto exprListNode = cursor.CurrentNode();
	if (exprListNode.GetType() != NodeName::kExpressionList ||
		exprListNode.GetChildCount() != 1)
	{
		ctx.symbols.PushBack(std::move(newSym));
		cursor.GoToParent(2);
		return;
	}

	if (!cursor.GoToFirstChild())
	{
		ctx.symbols.PushBack(std::move(newSym));
		cursor.GoToParent(2);
		return;
	}

	auto rhsExprNode = cursor.CurrentNode();
	newSym.type = GetSymTypeForRhsExpression(cursor, newSym.location, ctx);

	ctx.symbols.PushBack(std::move(newSym));

	if (rhsExprNode.GetType() == NodeName::kFunctionCall)
	{
		AddGlobalSymbolsForFunctionCall(cursor, ctx);
	}

	cursor.GoToParent(3);
}

void AddFunctionParameterSymbols(TreeCursor& cursor, 
							   LuaSourceDocumentSybolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	assert(cursor.CurrentNode().GetType() == NodeName::kParameters);

	if (!cursor.GoToFirstChild())
	{
		return;
	}

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

		ctx.symbols.PushBack(Sym{
			.name = paramName,
			.location = nodeLoc,
			.scope = { 
				nodeLoc.start, 
				ctx.scopeTree.GetCurrentScopeLocation().end 
			},
			.type = GetTableFunctionParamSymType(paramName, ctx),
			.kind = Sym::Kind::Parameter
		});

	} while (cursor.GoToNextNamedSibling());

	cursor.GoToParent();
}

void AddScopeReturnType(TreeCursor& cursor, 
					    LuaSourceDocumentSybolParseContext& ctx)
{
	assert(cursor.CurrentNode().GetType() == NodeName::kReturnStatement);

	const size_t parentFnScopeIdx = GetCurrentParentFunctionScopeIndex(ctx.scopeTree);
	if (parentFnScopeIdx >= ctx.scopeTree.Size())
	{
		return;
	}

	auto node = cursor.CurrentNode();
	if (node.GetNamedChildCount() == 1 && cursor.GoToFirstNamedChild())
	{
		auto exprNode = cursor.CurrentNode();

		const uint32_t retType = GetSymTypeForRhsExpression(
			cursor, exprNode.GetSourceLocation(), ctx);

		if (exprNode.GetType() == NodeName::kFunctionCall)
		{
			AddGlobalSymbolsForFunctionCall(cursor, ctx);
		}

		assert(ctx.scopeTree.CurrentValid());
		ctx.scopeTree.returns[parentFnScopeIdx] = retType;

		cursor.GoToParent();
	}
}

bool ParseLuaSourceDocumentSymbolsImpl(TreeCursor& cursor, 
									   LuaSourceDocumentSybolParseContext& ctx)
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

		if (NodeStartsNewScope(node))
		{
			ctx.scopeTree.PushScope(cursor);
		}
		else if (node.GetType() == NodeName::kVariableDeclaration)
		{
			AddSymForVariableDeclaration(cursor, ctx);
		}
		else if (node.GetType() == NodeName::kFunctionCall)
		{
			AddGlobalSymbolsForFunctionCall(cursor, ctx);
		}
		else if (node.GetType() == NodeName::kAssignmentStatement)
		{
			AddSymForAssignmentStatement(cursor, ctx);
		}
		else if (node.GetType() == NodeName::kReturnStatement)
		{
			AddScopeReturnType(cursor, ctx);
		}
		else if (node.GetType() == NodeName::kParameters)
		{
			AddFunctionParameterSymbols(cursor, ctx);
		}

		if (node.GetChildCount() > 0)
		{
			ParseLuaSourceDocumentSymbolsImpl(cursor, ctx);
		}

	} while (cursor.GoToNextSibling());

	cursor.GoToParent();

	return true;
}

bool ParseLuaSourceDocumentSymbols(TreeCursor& cursor,
								   LuaSourceDocumentSybolParseContext& ctx)
{
	if (!cursor)
	{
		return false;
	}

	assert(cursor.CurrentNode().GetType() == NodeName::kChunk);

	ctx.scopeTree.PushScope(cursor);

	return ParseLuaSourceDocumentSymbolsImpl(cursor, ctx);
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

SourceLocation LuaSourceDocumentScopeTree::GetCurrentScopeLocation() const
{
	if (!CurrentValid())
	{
		return {};
	}

	return locations[current];
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
