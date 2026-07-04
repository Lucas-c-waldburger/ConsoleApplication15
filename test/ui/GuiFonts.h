#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>

namespace ui {

struct GuiFonts
{
	ImFont* regular = nullptr;
	ImFont* medium = nullptr;
	ImFont* thin = nullptr;
	ImFont* bold = nullptr;
	ImFont* semiBold = nullptr;
};

} // ui

#endif
