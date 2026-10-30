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
		cursor.CurrentNode().GetType() != NodeName::kString)
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
						  const std::vector<SourceLocation>& scopeLocations)
{
	return scopeLocations.size() > 1 &&
		   node.GetSourceLocation().start > scopeLocations.back().end;
}

bool NodeEndsCurrentScope(const TreeNode& node, 
						  const LuaSourceDocumentScopeTree& scopeTree)
{
	return scopeTree.CurrentValid() && !scopeTree.AtRoot() &&
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

//size_t GetTopLevelFunctionScopeIndex(const LuaSourceDocumentSybolParseContext& ctx)
//{
//	assert(ctx.scopeNames.size() == ctx.scopeLocations.size());
//	assert(ctx.scopeNames.size() == ctx.scopeTypes.size());
//	assert(!ctx.scopeNames.empty());
//
//	using ScopeType = LuaSourceDocumentScopeType;
//
//	size_t lastFnScopeIdx = std::numeric_limits<size_t>::max();
//	
//	for (size_t i = ctx.scopeNames.size() - 1; i > 0; --i)
//	{
//		if (ctx.scopeTypes[i] == ScopeType::FunctionDeclaration ||
//			ctx.scopeTypes[i] == ScopeType::TableFunctionDeclaration)
//		{
//			lastFnScopeIdx = i;
//		}
//	}
//
//	return lastFnScopeIdx;
//}
//
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

std::optional<LuaSourceDocumentSymbol>
MakeSymForAssignmentStatement(TreeCursor& cursor, 
							  const LuaSourceDocumentSybolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kAssignmentStatement);

	Sym newSym{
		.scope = ctx.scopeTree.GetCurrentScopeLocation(),
		.assignment = node.GetSourceLocation()
	};

	if (node.GetNamedChildCount() != 2)
	{
		return std::nullopt;
	}
	if (!cursor.GoToFirstNamedChild())
	{
		return std::nullopt;
	}

	auto varListNode = cursor.CurrentNode();
	if (varListNode.GetType() != NodeName::kVariableList ||
		varListNode.GetChildCount() != 1) // only handle single var decls
	{
		cursor.GoToParent();
		return std::nullopt;
	}

	if (!cursor.GoToFirstNamedChild())
	{
		cursor.GoToParent();
		return std::nullopt;
	}
	
	auto lhsIdentNode = cursor.CurrentNode();
	if (lhsIdentNode.GetType() != NodeName::kIdentifier)
	{
		// skip inner type idexing assignment - only top level type reassignment
		cursor.GoToParent(2);
		return std::nullopt;
	}

	auto lhsText = lhsIdentNode.GetText(); // what are we reassigning
	if (lhsText.empty())
	{
		cursor.GoToParent(2);
		return std::nullopt;
	}

	newSym.name = lhsText;

	const size_t start = (ctx.symbols.Size() > 0) ? ctx.symbols.Size() - 1 : 0;
	bool found = false;
	for (size_t i = start; i <= 0; --i)
	{
		const auto [symName, symScope, symLoc, symType, symKind] = 
			ctx.symbols.GetView(i);

		if (symName != newSym.name)
		{
			continue;
		}

		if (newSym.assignment.Inside(symScope)) // if found symbol above in same scope
		{
			newSym.kind = symKind;
			found = true;

			break;
		}
	}

	cursor.GoToParent();

	if (!found)
	{
		cursor.GoToParent();
		return std::nullopt;
	}

	if (!cursor.GoToNextNamedSibling())
	{
		cursor.GoToParent();
		return newSym;
	}

	auto exprListNode = cursor.CurrentNode();
	if (exprListNode.GetType() != NodeName::kExpressionList ||
		exprListNode.GetChildCount() != 1)
	{
		cursor.GoToParent();
		return newSym;
	}

	if (!cursor.GoToFirstNamedChild())
	{
		cursor.GoToParent();
		return newSym;
	}

	auto rhsIdentNode = cursor.CurrentNode();
	if (rhsIdentNode.GetType() == NodeName::kIdentifier)
	{
		
	}
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

std::string_view 
GetScopeName(TreeCursor& cursor, std::string_view tableName,
			 const std::vector<std::string_view>& fnNames)
{
	if (cursor.CurrentNode().GetType() != NodeName::kFunctionDeclaration)
	{
		return NodeName::kBlock;
	}

	if (!cursor.GoToFirstNamedChild())
	{
		return {};
	}

	std::string_view result;
	do
	{
		auto node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kDotIndexExpression)
		{
			continue;
		}

		auto exprInfo = ExtractIndexExpressionInfo(node);
		if (!exprInfo)
		{
			continue;
		}
		assert(exprInfo->type == IndexExpressionType::Dot);

		if (exprInfo->lhsText != tableName)
		{
			continue;
		}

		if (std::ranges::find(fnNames, exprInfo->rhsText) != fnNames.end())
		{
			result = exprInfo->rhsText;
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

	const size_t origSymbolsSize = ctx.symbols.Size();
	for (size_t i = 0; i < origSymbolsSize; ++i)
	{
		const auto [symName, symScope, symLoc, symType, symKind] = 
			ctx.symbols.GetView(i);

		if (symLoc.start > scopeLoc.end)
		{
			break;
		}

		if (symKind != Sym::Kind::ScopedGlobal)
		{
			continue;
		}

		auto assignLoc = cursor.CurrentNode().GetSourceLocation();
		assignLoc.end = ctx.scopeTree.locations.front().end; // available until end of doc

		ctx.symbols.PushBack(Sym{
			.name = symName,
			.scope = ctx.scopeTree.GetCurrentScopeLocation(),
			.assignment = assignLoc,
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

SourceLocation 
GetScopeForDeclLocation(const SourceLocation& declLoc,
						const std::vector<SourceLocation>& scopeLocations)
{
	for (auto it = scopeLocations.rbegin(); it != scopeLocations.rend(); ++it)
	{
		if (declLoc.start >= it->start && declLoc.end <= it->end)
		{
			return *it;
		}
	}

	return {};
}

// Go through previously added symbols and find the one with same name & scope
uint32_t GetSymTypeForFoundIdentifier(std::string_view ident,
									  const SourceLocation& newSymLoc,
									  const LuaSourceDocumentSymbolSOA& symbols)
{
	for (size_t i = 0; i < symbols.Size(); ++i)
	{
		auto [symName, symScope, symDeclLoc, symType, _] = symbols.GetView(i);

		if (symName != ident)
		{
			continue;
		}

		if (newSymLoc.Inside(symScope) && newSymLoc.start > symDeclLoc.end)
		{
			return symType;
		}
	}

	return kInvalidLuaTypeId;
}

uint32_t GetSymTypeForIndexExpressionNode(const TreeNode& node, 
										  const SourceLocation& newSymLoc,
										  const LuaSourceDocumentSybolParseContext& ctx)
{
	auto exprInfo = ExtractIndexExpressionInfo(node);
	if (!exprInfo)
	{
		return kInvalidLuaTypeId;
	}

	// get type of lhs object from prev parsed symbols in current scope
	auto symType = GetSymTypeForFoundIdentifier(exprInfo->lhsText, newSymLoc, 
												ctx.symbols);
	if (symType == kInvalidLuaTypeId)
	{
		return kInvalidLuaTypeId;
	}

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

std::optional<LuaSourceDocumentSymbol>
MakeSymbolForVariableDeclaration(TreeCursor& cursor, 
								 LuaSourceDocumentSybolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

	auto node = cursor.CurrentNode();

	assert(node.GetType() == NodeName::kVariableDeclaration);
	assert(!ctx.scopeTree.CurrentValid());

	Sym newSym{
		.scope = ctx.scopeTree.GetCurrentScopeLocation(),
		.assignment = node.GetSourceLocation()
	};

	if (!cursor.GoToFirstChild())
	{
		return std::nullopt;
	}

	node = cursor.CurrentNode();
	if (node.GetText() == "local")
	{
		newSym.kind = Sym::Kind::Local;

		if (!cursor.GoToNextSibling())
		{
			cursor.GoToParent();
			return std::nullopt;
		}
	}
	else
	{
		newSym.kind = InsideFunctionScope(ctx.scopeTree) 
			? Sym::Kind::ScopedGlobal
			: Sym::Kind::UnscopedGlobal;
	}

	if (node.GetType() != NodeName::kAssignmentStatement ||
		node.GetNamedChildCount() != 2)
	{
		cursor.GoToParent();
		return std::nullopt;
	}

	if (!cursor.GoToFirstNamedChild())
	{
		cursor.GoToParent();
		return std::nullopt;
	}

	node = cursor.CurrentNode();
	if (node.GetChildCount() != 2)
	{
		cursor.GoToParent(2);
		return std::nullopt;
	}

	if (!cursor.GoToFirstChild())
	{
		cursor.GoToParent(2);
		return std::nullopt;
	}

	auto varListNode = cursor.CurrentNode();
	if (varListNode.GetType() != NodeName::kVariableList ||
		varListNode.GetChildCount() != 1) // only handle single var decls
	{
		cursor.GoToParent(3);
		return std::nullopt;
	}

	if (!cursor.GoToFirstChild())
	{
		cursor.GoToParent(3);
		return std::nullopt;
	}

	auto varIdentNode = cursor.CurrentNode();
	if (varIdentNode.GetType() != NodeName::kIdentifier)
	{
		cursor.GoToParent(4);
		return std::nullopt;
	}

	auto varName = varIdentNode.GetText();
	if (varName.empty())
	{
		cursor.GoToParent(4);
		return std::nullopt;
	}

	newSym.name = varName;

	cursor.GoToParent(2);
	if (!cursor.GoToNextNamedSibling())
	{
		cursor.GoToParent(2);
		return newSym;
	}

	auto exprListNode = cursor.CurrentNode();
	if (exprListNode.GetType() != NodeName::kExpressionList ||
		exprListNode.GetChildCount() != 1)
	{
		cursor.GoToParent(2);
		return newSym;
	}

	if (!cursor.GoToFirstChild())
	{
		cursor.GoToParent(2);
		return newSym;
	}

	auto exprNode = cursor.CurrentNode();
	if (exprNode.GetType() == NodeName::kNumber)
	{
		newSym.type = kNativeNumberLuaTypeId;
	}
	else if (exprNode.GetType() == NodeName::kBoolean)
	{
		newSym.type = kNativeBooleanLuaTypeId;
	}
	else if (exprNode.GetType() == NodeName::kString)
	{
		newSym.type = kNativeStringLuaTypeId;
	}
	else if (exprNode.GetType() == NodeName::kTableConstructor)
	{
		newSym.type = kNativeTableLuaTypeId;
	}
	else if (exprNode.GetType() == NodeName::kIdentifier)
	{
		auto ident = exprNode.GetText();

		newSym.type = GetSymTypeForFoundIdentifier(
			ident, newSym.assignment, ctx.symbols);
	}
	else if (exprNode.GetType() == NodeName::kFunctionCall)
	{
		if (exprNode.GetChildCount() == 1 && cursor.GoToFirstChild())
		{
			node = cursor.CurrentNode();
			if (IsIndexExpressionNode(node))
			{
				newSym.type = GetSymTypeForIndexExpressionNode(
					node, newSym.assignment, ctx);
			}

			cursor.GoToParent();
		}
	}

	cursor.GoToParent(3);

	return newSym;
}


bool ParseLuaSourceDocumentSymbolsImpl(TreeCursor& cursor, 
									   LuaSourceDocumentSybolParseContext& ctx)
{
	using Sym = LuaSourceDocumentSymbol;

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
			if (auto sym = MakeSymbolForVariableDeclaration(cursor, ctx))
			{
				ctx.symbols.PushBack(std::move(*sym));
			}
		}
		else if (node.GetType() == NodeName::kFunctionCall)
		{
			AddGlobalSymbolsForFunctionCall(cursor, ctx);
		}
		else if (node.GetType() == NodeName::kAssignmentStatement)
		{

		}
		else if (node.GetType() == NodeName::kParameters)
		{
			if (cursor.GoToFirstChild())
			{
				do
				{
					node = cursor.CurrentNode();
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

					ctx.symbols.PushBack(Sym{
						.name = paramName,
						.scope = ctx.scopeTree.GetCurrentScopeLocation(),
						.assignment = node.GetSourceLocation(),
						.type = GetTableFunctionParamSymType(paramName, ctx),
						.kind = Sym::Kind::Parameter
					});	

				} while (cursor.GoToNextNamedSibling());

				cursor.GoToParent();
			}
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

	if (current >= nodes.size())
	{
		return false;
	}

	auto treeNode = cursor.CurrentNode();
	assert(treeNode);

	const size_t newIdx = nodes.size();

	nodes[current].children.emplace_back(newIdx);
	locations.emplace_back(treeNode.GetSourceLocation());
	names.emplace_back(GetScopeName(cursor));

	auto& newScope = nodes.emplace_back();
	newScope.index = newIdx;
	newScope.parent = current;

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

//void LuaSourceDocumentSybolParseContext::PushScope(TreeCursor& cursor)
//{
//	auto node = cursor.CurrentNode();
//
//	scopeLocations.emplace_back(node.GetSourceLocation());
//	scopeNames.emplace_back(GetScopeName(cursor));
//	scopeTypes.emplace_back(
//		(node.GetType() == NodeName::kFunctionDeclaration
//			? !scopeNames.back().empty()
//				? LuaSourceDocumentScopeType::TableFunctionDeclaration
//				: LuaSourceDocumentScopeType::FunctionDeclaration
//			: LuaSourceDocumentScopeType::Block));
//}
//
//void LuaSourceDocumentSybolParseContext::PopScope()
//{
//	assert(scopeNames.size() == scopeLocations.size());
//	assert(scopeNames.size() == scopeTypes.size());
//	assert(scopeNames.size() > 0);
//
//	if (scopeNames.size() > 1)
//	{
//		scopeNames.pop_back();
//		scopeLocations.pop_back();
//		scopeTypes.pop_back();
//	}
//}


size_t GetLuaSourceDocumentSymbolIndex(std::string_view targetName,
									   const SourceLocation& usageLoc,
									   const LuaSourceDocumentSymbolSOA& symbols)
{
	using Sym = LuaSourceDocumentSymbol;

	size_t bestCandidate = std::numeric_limits<size_t>::max();

	for (size_t i = 0; i < symbols.Size(); ++i)
	{
		const auto [symName, symScope, symLoc, _, symKind] = symbols.GetView(i);

		if (symLoc.start > usageLoc.end)
		{
			break;
		}

		if (symName != targetName)
		{
			continue;
		}

		auto trueScope = symScope;
		if (symKind == Sym::Kind::Global)
		{
			trueScope.end = std::numeric_limits<size_t>::max();
		}
		
		if (usageLoc.Inside(trueScope))
		{
			bestCandidate = i;
		}
	}

	return bestCandidate;
}

} // ui

#endif
