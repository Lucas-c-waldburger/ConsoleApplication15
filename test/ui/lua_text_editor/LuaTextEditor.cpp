#include "LuaTextEditor.h"

#if IMGUI_ENABLED
#include <filesystem>
#include <fstream>
#include "LuaTextEditorCommon.h"
#include "TSEditorBridge.h"
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

    return kVoid;
}

Result<Void> LuaTextEditor::LoadScriptFile(std::string filepath,
                                           const ParsedLuaUserTypeInfo& userTypeInfo)
{
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
	AutoCompleteUtility::ResourceContext autoCompleteCtx{ textEditor_, parser_, userTypeInfo };

    if (!textEditor_.IsGhostTextVisible())
    {
        parser_.ClearCompletionMarkers();
    }

    if (textEditor_.IsTextChanged())
    {
        auto newSource = textEditor_.GetText();

		UpdateParserCompletionMarkers(newSource);

        parser_.EditSource(std::move(newSource));
    
        autoCompleteUtility_.UpdateForNewText(autoCompleteCtx);
    }
    if (textEditor_.IsCursorPositionChanged())
    {
        autoCompleteUtility_.UpdateForNewCursorPos(autoCompleteCtx);
    }

    autoCompleteUtility_.Draw(autoCompleteCtx);

    textEditor_.Render(title.data(), aSize, aBorder);

    autoCompleteUtility_.Cleanup(autoCompleteCtx);
}

void LuaTextEditor::SetReadOnly(bool rOnly)
{
    textEditor_.SetReadOnly(rOnly);
}

bool LuaTextEditor::IsReadOnly() const
{
    return textEditor_.IsReadOnly();
}

void LuaTextEditor::Clear()
{
    autoCompleteUtility_.Reset(textEditor_);
    textEditor_.SetAllKeysHandled(true);
    textEditor_.SetText("");
    loadedFilepath_.clear();
    parser_.Reset();
}

std::string LuaTextEditor::GetWordUnderCursor() const
{
    return textEditor_.GetWordUnderCursor();
}

void LuaTextEditor::UpdateParserCompletionMarkers(std::string_view newSource)
{
    parser_.ClearCompletionMarkers();

    const auto cursorByteOffset = TextPositionToByteOffset(newSource,
                                                           textEditor_.GetCursorPosition(),
                                                           textEditor_.GetTabSize());

    auto autoCompData = ObjectFieldAutoCompleteData::Parse(newSource, cursorByteOffset);
    if (autoCompData.IsValid())
    {
        parser_.AddCompletionMarker(autoCompData.expressionStartByte);
    }
}

//void LuaTextEditor::ResolveDocumentParseErrors()
//{
//    if (!textEditor_.IsGhostTextVisible())
//    {
//        LOG_WARNING("Could not resolve document parse errors: "
//            "ghost text is not visible");
//
//        return;
//    }
//
//    const auto& autoCompData = autoCompleteUtility_.GetAutoCompleteData();
//    if (!autoCompData.IsValid())
//    {
//        LOG_WARNING("Could not resolve document parse errors: "
//            "auto-complete data is invalid");
//
//        return;
//    }
//
//    const auto cursorByteOffset = TextPositionToByteOffset(parser_.GetSourceString(),
//                                                           textEditor_.GetCursorPosition(),
//                                                           textEditor_.GetTabSize());
//    if (cursorByteOffset != autoCompData.expressionStartByte)
//    {
//        LOG_WARNING("Could not resolve document parse errors: "
//            "text editor cursor is not at expression start byte");
//
//        return;
//    }
//
//    auto& parsedDoc = parser_.GetSourceDocument(); 
//    for (const auto& err : parsedDoc.errors)
//    {
//        
//    }
//}

} // ui

#endif
