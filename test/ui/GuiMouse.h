#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../ecs/Ecs.h"
#include "../../core/Result.h"
#include "../../core/commonObjects.h"
#include "InspectorCommon.h"

class SceneFixture;

namespace ui {

class GuiMouse
{
public:
	//static void Init(int renderW, int renderH, SDL_FRect displayArea);
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

	//static void SetDisplayArea(SDL_FRect displayArea);

	static void EvaluateInsideWindow(EditorWindowType windowType);

	//static void Update(const SceneFixture& fixture);
	static void Reset() { insideWindowType_.reset(); }

	static const std::optional<EditorWindowType>& GetInsideWindowType() { return insideWindowType_; }

private:
	GuiMouse() = default;

	static bool CheckState(MouseInputSource src, InputState st);

	static SDL_FPoint ToRenderTarget(SDL_FPoint p);
	 
	static inline Entity entity_;
	//static inline Dimensions<int> renderTargetDimensions_;
	//static inline SDL_FRect displayArea_;
	static inline std::optional<EditorWindowType> insideWindowType_;
};

} // ui

#endif