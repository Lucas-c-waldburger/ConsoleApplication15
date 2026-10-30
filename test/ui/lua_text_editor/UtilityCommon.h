#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED

class ParsedLuaUserTypeInfo;
class TextEditor;

namespace ui {

class LuaSourceParser;

struct LuaTextEditorResourceContext
{
	TextEditor& textEditor;
	const LuaSourceParser& parser;
	const ParsedLuaUserTypeInfo& userTypeInfo;
};

} // ui

#endif