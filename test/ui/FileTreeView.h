#pragma once
#include <imgui.h>
#include "../../file/FilePathUtility.h"

namespace ui {

namespace fs = std::filesystem;

class FileTreeView
{
public:
	struct State
	{
		fs::path currentPath = FilePathUtility::GetRootPath();
		std::optional<fs::path> selectedFile;
	};




private:
	State state_;
};









}