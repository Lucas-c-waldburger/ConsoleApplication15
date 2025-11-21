#pragma once
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
			const auto& path = entry.path();
			const std::string name = entry.path().filename().string();
			const bool isDir = entry.is_directory();

			if (isDir)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
			}

			bool selected = (state.selectedFile.has_value() && 
							 *state.selectedFile == path);

			if (ImGui::Selectable(name.c_str(), selected))
			{
				if (isDir)
				{
					state.currentPath = path;
					state.selectedFile.reset();

				}
				else
				{
					state.selectedFile = path;
				}
			}

			//if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)
			//	&& state.selectedFile.has_value();

			//if (isDir)
			//{
			//	ImGui::PopStyleColor();
			//}
		}

		//if ((ImGui::Button("Open") || )

	}


private:
	State state_;
};









}