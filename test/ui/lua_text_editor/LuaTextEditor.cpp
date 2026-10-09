#include "LuaTextEditor.h"

#if IMGUI_ENABLED
#include <filesystem>
#include <fstream>
#include "LuaTextEditorCommon.h"
#include "TSEditorBridge.h"
#include "AutoCompleteUtility.h"
#include "../../scripting/ParsedLuaUserTypeInfo.h"

namespace ui {

namespace {

} // unnamed

LuaTextEditor::LuaTextEditor()
{
    textEditor_.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
}

Result<Void> LuaTextEditor::LoadScriptString(std::string source,
                                             const ParsedLuaUserTypeInfo& userTypeInfo)
{
    loadedFilepath_.clear();
    textEditor_.SetText(source);
	parser_.Parse(std::move(source));
	autoCompleteDisplay_.UpdateForNewText({ textEditor_, parser_, userTypeInfo });

    return kVoid;
}

Result<Void> LuaTextEditor::LoadScriptFile(std::string filepath,
                                           const ParsedLuaUserTypeInfo& userTypeInfo)
{
    autoCompleteDisplay_.Reset(textEditor_);
    loadedFilepath_.clear();
    parser_.Reset();

    std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        return MAKE_ERROR_FMT("Could not open file at path '{}'", filepath);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string content;
    content.resize(size);

    if (!file.read(content.data(), size))
    {
        return MAKE_ERROR_FMT("Error reading file at path '{}'", filepath);
    }

    textEditor_.SetText(content);

    loadedFilepath_ = std::move(filepath);
	parser_.Parse(std::move(content));
    autoCompleteDisplay_.UpdateForNewText({ textEditor_, parser_, userTypeInfo });
    //parser_.PrintSyntaxTree();

    return kVoid;
}

Result<Void> LuaTextEditor::SaveScriptFile(std::string_view destPath)
{
    std::ofstream file;
    if (!destPath.empty())
    {
        file.open(destPath.data(), std::ios::out | std::ios::binary);
    }
    else
    {
        file.open(destPath, std::ios::out | std::ios::binary);
    }

    if (!file.is_open())
    {
        std::string_view attemptedPath = (!destPath.empty()) 
            ? destPath 
            : std::string_view{ loadedFilepath_ };

        return MAKE_ERROR_FMT("Could not open file at path '{}'", attemptedPath);
    }

    auto data = textEditor_.GetText();

    file.write(data.data(), data.size());

    if (!file.good())
    {
        std::string_view attemptedPath = (!destPath.empty())
            ? destPath
            : std::string_view{ loadedFilepath_ };

        return MAKE_ERROR_FMT("Error writing file at path '{}'", attemptedPath);
    }

    return kVoid;
}

void LuaTextEditor::Render(std::string_view title, const ImVec2& aSize, bool aBorder)
{
    textEditor_.Render(title.data(), aSize, aBorder);
}

void LuaTextEditor::Update(const ParsedLuaUserTypeInfo& userTypeInfo,
                           std::string_view title, const ImVec2& aSize, bool aBorder)
{
	AutoCompleteDisplay::ResourceContext autoCompleteCtx{ textEditor_, parser_, userTypeInfo };

    if (textEditor_.IsTextChanged())
    {
         auto newSource = textEditor_.GetText();
         parser_.EditSource(std::move(newSource));
    
         autoCompleteDisplay_.UpdateForNewText(autoCompleteCtx);
    }
    if (textEditor_.IsCursorPositionChanged())
    {
        autoCompleteDisplay_.UpdateForNewCursorPos(autoCompleteCtx);
    }

	autoCompleteDisplay_.Render(autoCompleteCtx);

    textEditor_.Render(title.data(), aSize, aBorder);

    autoCompleteDisplay_.Cleanup(autoCompleteCtx);
}

void LuaTextEditor::SetReadOnly(bool rOnly)
{
    //autoCompleteDisplay_.Reset(textEditor_);

    textEditor_.SetReadOnly(rOnly);
}

bool LuaTextEditor::IsReadOnly() const
{
    return textEditor_.IsReadOnly();
}

void LuaTextEditor::Clear()
{
    //autoCompleteDisplay_.Reset(textEditor_);
    textEditor_.SetText("");
    loadedFilepath_.clear();
    parser_.Reset();
}

std::string LuaTextEditor::GetWordUnderCursor() const
{
    return textEditor_.GetWordUnderCursor();
}

} // ui

#endif
