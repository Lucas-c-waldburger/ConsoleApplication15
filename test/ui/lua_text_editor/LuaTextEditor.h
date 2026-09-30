#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../deps/ImGuiColorTextEdit/TextEditor.h"
#include "../../core/Result.h"
#include "../../core/commonObjects.h"

namespace ui {

class LuaTextEditor
{
public:
	LuaTextEditor()
	{
		textEditor_.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
	}
	
	Result<Void> LoadScriptFile(std::string filepath);

	Result<Void> SaveScriptFile(std::string_view destPath = "");

	void Render(std::string_view title, const ImVec2& aSize = ImVec2(), bool aBorder = false);

	void SetReadOnly(bool rOnly);

	bool IsReadOnly() const;

	void Clear();

	const std::string& GetLoadedFilepath() const noexcept { return loadedFilepath_; }

	std::string GetWordUnderCursor() const;

private:
	TextEditor textEditor_;
	std::string loadedFilepath_;
};


} // ui

#endif