#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../ecs/Ecs.h"
#include "../../core/Result.h"
#include "../../core/commonObjects.h"

namespace ui {

class GuiMouse
{
public:
	static void Init();

	static SDL_FPoint GetPosition();
	static SDL_FPoint GetRelativePosition();

	static bool IsLeftClicked();
	static bool IsLeftHeld();
	static bool IsLeftReleased();

	static bool IsRightClicked();
	static bool IsRightHeld();
	static bool IsRightReleased();

	static bool IsMiddleClicked();
	static bool IsMiddleHeld();
	static bool IsMiddleReleased();

	static bool IsWheelScrolled();
	static float GetScrollY();

	static bool InsideEditorWindow();
private:
	GuiMouse() = default;

	static bool CheckState(MouseInputSource src, InputState st);
	 
	static inline Entity entity_;
};

} // ui

#endif