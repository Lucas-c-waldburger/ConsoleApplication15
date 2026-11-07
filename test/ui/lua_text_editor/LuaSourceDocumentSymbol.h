#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "LuaSourceParserCommon.h"
#include "../../core/StableSOA.h"
#include "../../scripting/ParsedLuaUserTypeInfo.h"
#include "ObjectFieldAutoCompleteData.h"
#include <vector>

namespace ui {

struct TreeCursor;

struct LuaSourceDocumentScope
{
	size_t index = std::numeric_limits<size_t>::max();
	size_t parent = std::numeric_limits<size_t>::max();
	std::vector<size_t> children;

	bool IsLeaf() const { return children.empty(); }
};

struct LuaSourceDocumentScopeTree
{
	size_t current = 0;

	void Reset() { current = 0; }
	bool GoToParent();

	bool PushScope(TreeCursor& cursor);
	bool AtRoot() const noexcept { return current <= 0; }
	bool AtLeaf() const noexcept { return current < nodes.size() && nodes[current].IsLeaf(); }
	bool CurrentValid() const;
	bool SetCurrent(size_t idx);

	size_t Size() const noexcept;

	SourceLocation GetCurrentScopeLocation() const;
	std::string_view GetCurrentScopeName() const;
	size_t GetDocumentEnd() const;

	std::vector<SourceLocation> locations;
	std::vector<std::string_view> names;
	std::vector<uint32_t> returns;
	std::vector<LuaSourceDocumentScope> nodes;
};

struct LuaSourceDocumentSymbol
{
	enum class Kind
	{
		Unknown,
		Parameter,
		Local,
		ScopedGlobal,
		UnscopedGlobal
	};

	std::string_view name;
	SourceLocation location;
	SourceLocation scope;
	uint32_t type = std::numeric_limits<uint32_t>::max();
	Kind kind = Kind::Unknown;

	constexpr bool operator==(const LuaSourceDocumentSymbol&) const = default;
};

inline constexpr LuaSourceDocumentSymbol kInvalidSourceDocumentSymbol{};

using LuaSourceDocumentSymbolSOA = StableSOA<
	LuaSourceDocumentSymbol,
	&LuaSourceDocumentSymbol::name,
	&LuaSourceDocumentSymbol::location,
	&LuaSourceDocumentSymbol::scope,
	&LuaSourceDocumentSymbol::type,
	&LuaSourceDocumentSymbol::kind
>;

struct LuaSourceDocumentTableInfo
{
	std::string_view functionTableName;
	std::string_view metaTableName;
};

struct LuaSourceDocumentSignaturesTable
{
	std::vector<std::string_view> functionNames;
	// may not actually need parameter names
	std::vector<std::vector<std::string_view>> parameterNames;
	std::vector<std::vector<uint32_t>> parameterTypes;
};

struct LuaSourceDocumentSymbolParseContext
{
	LuaSourceDocumentSymbolSOA symbols;
	const ParsedLuaUserTypeInfo& userTypeInfo;
	LuaSourceDocumentTableInfo tableInfo;
	LuaSourceDocumentSignaturesTable signaturesTable;
	LuaSourceDocumentScopeTree scopeTree;
	std::vector<SourceDocumentError> errors;

	void AddError(const TreeNode& node, std::string_view msg);
};

bool ParseLuaSourceDocumentSymbols(TreeCursor& cursor,
	LuaSourceDocumentSymbolParseContext& ctx);

size_t FindMatchingSymbolIndex(std::string_view targetName, 
							   size_t byteOffset,
							   LuaSourceDocumentSymbolSOA& symbols);

size_t FindMatchingSymbolIndex(std::string_view targetName,
							   SourceLocation targetLoc,
							   LuaSourceDocumentSymbolSOA& symbols);

} // ui

#endif