#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "LuaSourceParserCommon.h"
#include "../../core/StableSOA.h"
#include "TreeCursor.h"

namespace ui {

struct NodeKey
{
	SourceLocation location;
	TSSymbol tsSymbol;
};

} // ui

namespace std {

template <>
struct hash<ui::NodeKey> {
	size_t operator()(const ui::NodeKey& k) const noexcept {
		return MakeHash(k.location, k.tsSymbol);
	};
};

} // std

namespace ui {

namespace detail {

struct UnresolvedLuaTypeId;

} // detail

static constexpr uint32_t kUnresolvedLuaTypeId = 
	TypeInfo<detail::UnresolvedLuaTypeId>::hash32;

using ScopeID = uint32_t;
using SymbolID = uint32_t;
using ValueID = uint32_t;
using FunctionID = uint32_t;
using SuperTypeID = uint32_t;

enum class IdType
{
	Invalid,
	Scope,
	Symbol,
	Value,
	Function
};

struct DocID
{
	IdType type;
	uint32_t value;
};

struct ScopeNode
{
	ScopeID index = Null<ScopeID>();
	ScopeID parent = Null<ScopeID>();
	std::vector<ScopeID> children;
};

enum class ScopeKind
{
	Invalid,
	Chunk,
	Function,
	Block
}; 

enum class ScopeClass
{
	Local,
	ScopedGlobal,
	UnscopedGlobal
};

enum class SymbolSuperType
{
	Value,
	UserType,
	Function,
	Table,
	Parameter
};

enum class SymbolBindingType
{
	Declaration,
	Assignment
};

struct TableObjectData
{
	std::vector<SymbolID> symbolIds;
};

struct Symbol
{
	std::string_view name;
	SourceLocation location;
	SourceLocation visibility;
	uint32_t type = kUnresolvedLuaTypeId;
	SymbolSuperType super = SymbolSuperType::Value;
	SymbolBindingType binding = SymbolBindingType::Declaration;
	ScopeClass scopeClass = ScopeClass::Local;
	ScopeID scopeId = Null<ScopeID>();
};

enum class ParseContextType
{
	VariableDeclaration,
	AssignmentStatement
};

// will we have to reverse-search in same scope if sym type was reassigned,
// and then change visibility of the prev symbol to stop before the reassignment?

// vectors should be sorted by appearance in document
struct ScopeTree
{
	enum Flag : uint8_t
	{
		FunctionInvoked = 1 << 0
	};

	std::vector<ScopeKind> kinds;
	std::vector<SourceLocation> locations;
	std::vector<TreeNode> contents;
	std::vector<ScopeNode> scopes;
	std::vector<ScopeClass> classes;
	std::vector<uint32_t> resultTypes;
	std::vector<uint8_t> flags;
	std::vector<Range<SymbolID>> symbolRanges;

	ScopeID current = Null<ScopeID>();

	static ScopeTree Parse(TreeCursor& cursor);

	bool SymInsideScope(SourceLocation symLoc, ScopeID scopeId) const;
	ScopeID FindEnclosingFunctionScope(ScopeID scopeId) const;
	size_t GetCurrentScopeEnd() const;
	size_t GetDocumentEnd() const;

	size_t Size() const;
};

using SymbolSOA = StableSOA<
	Symbol,
	&Symbol::name,
	&Symbol::location,
	&Symbol::visibility,
	&Symbol::type,
	&Symbol::super,
	&Symbol::binding,
	&Symbol::scopeClass,
	&Symbol::scopeId
>;

struct ScopeTreeParseContext
{
	std::vector<TreeNode> contents;
	std::vector<uint8_t> flags;
	Range<SymbolID> symbolRange;
	ScopeID current = Null<ScopeID>();
};

struct VariableDeclarations // Actually type hints
{
	std::vector<std::string_view> typeText;
	std::vector<SourceLocation> typeSpans;
	std::vector<SymbolID> symbolIds;
	std::vector<TreeCursor> rhsCursors;

	size_t AddEntry();
	size_t Size() const;
};

struct AssignmentStatements // actually multi-indexed 
{
	std::vector<std::vector<std::string_view>> identifiers;
	std::vector<std::vector<IndexExpressionType>> indexExprTypes;
	std::vector<SymbolID> symbolIds;
	std::vector<TreeCursor> rhsCursors; // blank if made from function

	size_t AddEntry();
	void PopEntry();
	size_t Size() const;
};

struct SymbolResolutionList
{
	std::vector<ParseContextType> ctxType;
	std::vector<size_t> index;
};

struct LuaSourceDocParseContext
{
	ScopeTree& scopeTree;
	SymbolSOA& symbols;
	VariableDeclarations declarations;
	AssignmentStatements assignments;
	std::vector<SourceDocumentError> errors;

	void AddError(const TreeNode& node, std::string_view msg);
	void AddError(const TreeCursor& cursor, std::string_view msg);
};

void ParseLuaSourceDoc(TreeNode& node, LuaSourceDocParseContext& ctx);
void ParseLuaSourceDocImpl(TreeNode& node, LuaSourceDocParseContext& ctx);

inline bool NeedCreateUnscopedGlobal(ScopeClass symClass, ScopeID scopeId,
									 const ScopeTree& tree)
{
	if (symClass != ScopeClass::ScopedGlobal)
	{
		return false;
	}

	const auto fnScopeId = tree.FindEnclosingFunctionScope(scopeId);

	return fnScopeId < tree.Size() &&
		   tree.kinds[fnScopeId] == ScopeKind::Function &&
		   (tree.flags[fnScopeId] & ScopeTree::Flag::FunctionInvoked) == 0;
}

inline size_t GetSymbolVisibilityEnd(SourceLocation symLoc,
									 ScopeID scopeId, const ScopeTree& tree)
{
	if (!tree.SymInsideScope(symLoc, scopeId))
	{
		return Null<size_t>();
	}

	return tree.locations[scopeId].end;
}

} // ui

#endif