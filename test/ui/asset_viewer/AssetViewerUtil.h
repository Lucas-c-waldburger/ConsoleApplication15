#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>

namespace ui {

void DrawCheckerboard(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags, 
					  const ImVec2& size_arg, float grid_step);


} // ui

#endif