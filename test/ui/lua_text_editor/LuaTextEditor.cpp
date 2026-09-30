#include "LuaTextEditor.h"

#if IMGUI_ENABLED
#include <filesystem>
#include <fstream>

namespace ui {

Result<Void> LuaTextEditor::LoadScriptFile(std::string filepath)
{
    loadedFilepath_.clear();

    std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        return MAKE_ERROR_FMT("Could not open file at path '{}'", filepath);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg); // Rewind back to the start

    std::string contents;
    contents.resize(size); // Allocate memory upfront to prevent reallocations

    if (!file.read(contents.data(), size)) 
    {
        return MAKE_ERROR_FMT("Error reading file at path '{}'", filepath);
    }

    textEditor_.SetText(contents);

    loadedFilepath_ = std::move(filepath);

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
    textEditor_.SetText("");
    loadedFilepath_.clear();
}

std::string LuaTextEditor::GetWordUnderCursor() const
{
    return textEditor_.GetWordUnderCursor();
}

} // ui

#endif
