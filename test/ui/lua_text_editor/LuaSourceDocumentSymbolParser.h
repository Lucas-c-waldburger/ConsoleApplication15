#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "LuaSourceDocumentScopeTree.h"
#include "LuaSourceDocumentSymbol2.h"
#include "../../scripting/ParsedLuaUserTypeInfo.h"

namespace ui::lua {

struct LuaSourceDocumentParseContext
{
	LuaSourceDocumentSymbolSOA& symbols;
	LuaSourceDocumentScopeTree& scopeTree;
	ParsedLuaUserTypeInfo& userTypeInfo;
	std::vector<SourceLocation> eraseLocations;
	std::vector<SourceDocumentError> errors;

	void AddError(const TreeNode& node, std::string_view msg);
	void AddError(const TreeCursor& cursor, std::string_view msg);
};


} // ui::lua

#endif