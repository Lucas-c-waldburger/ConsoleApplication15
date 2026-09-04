#pragma once
#include "../../FeatureFlags.h"

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

    FileTreeView() = default;
    FileTreeView(const fs::path& defaultPath) { SetDirectory(defaultPath); }

    bool Display()
    {
        assert(fs::exists(state_.currentPath));

        bool complete = false;

        if (ImGui::Selectable(".."))
        {
            if (state_.currentPath != state_.topLevelPath &&
                state_.currentPath.has_parent_path())
            {
                state_.currentPath = state_.currentPath.parent_path();
            }
        }

        for (auto& entry : fs::directory_iterator(state_.currentPath))
        {
            const fs::path path = entry.path();
            const std::string name = path.filename().string();
            const bool isDir = entry.is_directory();

            if (isDir)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
            }

            bool selected = (state_.selectedFile.has_value() &&
                             *state_.selectedFile == path);

            bool activated = ImGui::Selectable(name.c_str(), selected);

            bool doubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0);

            if (activated || doubleClicked)
            {
                if (isDir) 
                {
                    // Enter folder
                    state_.currentPath = path;
                    state_.selectedFile.reset();
                }
                else 
                {
                    // Single click: select file
                    state_.selectedFile = path;

                    // Double-click: confirm selection -> return true
                    complete = doubleClicked;
                }
            }

            if (isDir)
            {
                ImGui::PopStyleColor();
            }
        }

        if (!complete && state_.selectedFile.has_value())
        {
            complete = (ImGui::Button("Open"));
        }

        return complete;
    }

    void SetDirectory(const fs::path& path)
    {
        assert(fs::exists(path));

        state_.topLevelPath = path;
        state_.currentPath = path;
    }

    auto GetCurrentDirectoryIter()
    {
        assert(fs::exists(state_.currentPath));

        return fs::directory_iterator(state_.currentPath);
    }

    bool HasSelectedFile() const
    {
        return state_.selectedFile.has_value();
    }

    std::string GetSelectedFile() const
    {
        return (HasSelectedFile()) ? state_.selectedFile->string() : "";
    }

private:
    State state_;
};

}

#endif