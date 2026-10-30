#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "LuaSourceParser.h"
#include "UtilityCommon.h"
#include "../../deps/ImGuiColorTextEdit/TextEditor.h"

namespace ui {

class TextDecorationUtility
{
public:
	static constexpr TextEditor::Palette kUserPalette{
		IM_COL32(220, 220, 154, 255), // Function
		IM_COL32(241, 241, 241, 255), // Property
		IM_COL32(156, 220, 254, 255), // Variable
		IM_COL32(62, 144, 214, 255), // Local
	};

	enum class UserPaletteIndex : uint8_t
	{
		Function = 0,
		Property = 1,
		Variable = 2,
		Local = 3
	};

	void Update(const LuaTextEditorResourceContext& ctx);
	void Reset(TextEditor& textEditor);

private:
	struct PendingError
	{
		SourceDocumentError error;
		float cursorInRegionDelta = 0.0f;
	};

	void UpdateForNoChange(const LuaTextEditorResourceContext& ctx);
	void UpdateForNewText(const LuaTextEditorResourceContext& ctx);
	void UpdateForNewCursorPos(const LuaTextEditorResourceContext& ctx);

	void DrawDocumentErrorDecorations(const LuaSourceParser& parser, TextEditor& textEditor);

	std::optional<PendingError> pendingError_;
	size_t cursorBytePos_ = 0;
};


} // ui

#endif