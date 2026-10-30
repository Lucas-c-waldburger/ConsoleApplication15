#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "LuaSourceParserCommon.h"
#include "../../core/StableSOA.h"
#include "../../scripting/ParsedLuaUserTypeInfo.h"
#include <vector>

namespace ui {

class TreeCursor;

enum class LuaSourceDocumentScopeType : uint8_t
{
	FunctionDeclaration,
	TableFunctionDeclaration,
	Block
};

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
	bool CurrentValid() const;
	bool SetCurrent(size_t idx);

	size_t Size() const noexcept;

	SourceLocation GetCurrentScopeLocation() const;
	std::string_view GetCurrentScopeName() const;

	std::vector<SourceLocation> locations;
	std::vector<std::string_view> names;
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
	SourceLocation scope;
	SourceLocation assignment;
	uint32_t type = std::numeric_limits<uint32_t>::max();
	Kind kind = Kind::Unknown;

	constexpr bool operator==(const LuaSourceDocumentSymbol&) const = default;
};

inline constexpr LuaSourceDocumentSymbol kInvalidSourceDocumentSymbol{};

using LuaSourceDocumentSymbolSOA = StableSOA<
	LuaSourceDocumentSymbol,
	&LuaSourceDocumentSymbol::name,
	&LuaSourceDocumentSymbol::scope,
	&LuaSourceDocumentSymbol::assignment,
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
	std::vector<std::vector<std::string_view>> parameterNames;
	std::vector<std::vector<uint32_t>> parameterTypes;
};

struct LuaSourceDocumentSybolParseContext
{
	LuaSourceDocumentSymbolSOA& symbols;
	ParsedLuaUserTypeInfo& userTypeInfo;
	LuaSourceDocumentTableInfo& tableInfo;
	LuaSourceDocumentSignaturesTable& signaturesTable;
	//std::vector<SourceLocation> scopeLocations;
	//std::vector<std::string_view> scopeNames;
	//std::vector<LuaSourceDocumentScopeType> scopeTypes;
	LuaSourceDocumentScopeTree scopeTree;

	//void PushScope(TreeCursor& cursor);
	//void PopScope();
};

struct LuaSourceDocumentSymbolTable
{
	LuaSourceDocumentSymbolSOA symbols;
	std::vector<SourceLocation> scopeLocations;
	std::vector<size_t> scopeIndexRanges;
};

size_t GetLuaSourceDocumentSymbolIndex(std::string_view targetName, 
									   const SourceLocation& usageLoc,
									   const LuaSourceDocumentSymbolSOA& symbols);



} // ui

#endif