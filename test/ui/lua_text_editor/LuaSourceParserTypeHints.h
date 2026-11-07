#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "LuaSourceParserCommon.h"
#include "TreeCursor.h"
#include "../../scripting/ParsedLuaUserTypeInfo.h"

namespace ui {

enum class VariableScopeType
{
	Parameter,
	Local,
	ScopedGlobal,
	UnscopedGlobal
};

struct ParsedVariableInfo
{
	std::string_view name;
	std::string_view typeText;
	VariableScopeType scopeType;
	SourceLocation errorSpan;
	TreeCursor rhsCursor;
};

} // ui

#endif