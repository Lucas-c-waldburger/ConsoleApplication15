#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../deps/ImGuiColorTextEdit/TextEditor.h"
#include "../../core/Result.h"
#include "../../core/commonObjects.h"
#include "LuaSourceParser.h"
#include "AutoCompleteUtility.h"

class ParsedLuaUserTypeInfo;

namespace ui {

class LuaTextEditor
{
public:
	LuaTextEditor();
	~LuaTextEditor() = default;
	LuaTextEditor(const LuaTextEditor&) = delete;
	LuaTextEditor& operator=(const LuaTextEditor&) = delete;
	LuaTextEditor(LuaTextEditor&&) noexcept = default;
	LuaTextEditor& operator=(LuaTextEditor&&) noexcept = default;
	
	Result<Void> LoadScriptString(std::string source, 
								  const ParsedLuaUserTypeInfo& userTypeInfo);

	Result<Void> LoadScriptFile(std::string filepath,
								const ParsedLuaUserTypeInfo& userTypeInfo);

	Result<Void> SaveScriptFile(std::string_view destPath = "");

	void Update(const ParsedLuaUserTypeInfo& userTypeInfo, std::string_view title,
				const ImVec2& aSize = ImVec2(), bool aBorder = false);

	void Render(std::string_view title, const ImVec2& aSize = ImVec2(), bool aBorder = false);

	void SetReadOnly(bool rOnly);

	bool IsReadOnly() const;

	void Clear();

	const std::string& GetLoadedFilepath() const noexcept { return loadedFilepath_; }

	std::string GetWordUnderCursor() const;

	LuaSourceParser& GetParser() noexcept { return parser_; }
	TextEditor& GetTextEditor() noexcept { return textEditor_; }

private:
	TextEditor textEditor_;
	LuaSourceParser parser_;
	std::string loadedFilepath_;
	AutoCompleteDisplay autoCompleteDisplay_;
};


} // ui

#endif