#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "GuiFonts.h"
#include "../../core/commonObjects.h"
#include "../../core/Result.h"
//#include "GuiTexture.h"
//#include "../Fixtures.h"

namespace ui {

class GuiResource
{
public:
	static Result<Void> Init();

	static const GuiFonts& Fonts() { return fonts_; }

private:
	GuiResource() = default;

	static inline GuiFonts fonts_{};
};

} // ui

#endif