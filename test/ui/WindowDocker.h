#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../core/commonObjects.h"
#include "Editor.h"

namespace ui {

class WindowDocker
{
public:
	static constexpr float kDefaultWindowWidth = 400.0f;

	void Update()
	{
		const auto availSpace = ImGui::GetContentRegionAvail();

		
	}

private:
	std::unordered_map<Editor::WindowType, ImVec4> windowPositions_;
};


} // ui

#endif