#pragma once

#if IMGUI_ENABLED

#include "../../systems/GuiSystem.h"
#include "../../file/FilePathUtility.h"
#include <format>

namespace ui {

namespace fs = std::filesystem;

class FileTreeView
{
public:
	struct State
	{
		fs::path topLevelPath = FilePathUtility::GetRootPath();
		fs::path currentPath = FilePathUtility::GetRootPath();
		std::optional<fs::path> selectedFile;
	};

    bool Display(State& state)
    {
        assert(fs::exists(state.currentPath));

        bool complete = false;

        if (ImGui::Selectable(".."))
        {
            if (state.currentPath != state.topLevelPath &&
                state.currentPath.has_parent_path())
            {
                state.currentPath = state.currentPath.parent_path();
            }
        }

        for (auto& entry : fs::directory_iterator(state.currentPath))
        {
            const fs::path path = entry.path();
            const std::string name = path.filename().string();
            const bool isDir = entry.is_directory();

            if (isDir)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
            }

            bool selected = (state.selectedFile.has_value() &&
                             *state.selectedFile == path);

            bool activated = ImGui::Selectable(name.c_str(), selected);

            bool doubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0);

            if (activated || doubleClicked)
            {
                if (isDir) 
                {
                    // Enter folder
                    state.currentPath = path;
                    state.selectedFile.reset();
                }
                else 
                {
                    // Single click: select file
                    state.selectedFile = path;

                    // Double-click: confirm selection Å® return true
                    complete = doubleClicked;
                }
            }

            if (isDir)
            {
                ImGui::PopStyleColor();
            }
        }

        if (!complete && state.selectedFile.has_value())
        {
            complete = (ImGui::Button("Open"));
        }

        return complete;
    }
};

}

#endif