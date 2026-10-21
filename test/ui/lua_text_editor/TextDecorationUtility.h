#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "LuaSourceParser.h"
#include "../../deps/ImGuiColorTextEdit/TextEditor.h"

namespace ui {

class TextDecorationUtility
{
public:
	void UpdateForNewText(const LuaSourceParser& parser, TextEditor& textEditor);
	void Reset(TextEditor& textEditor);

private:
	void DrawErrors(const LuaSourceParser& parser, TextEditor& textEditor);
};


} // ui

#endif