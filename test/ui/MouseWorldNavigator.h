#pragma once

#if IMGUI_ENABLED

#include "DataEditDisplayUtils.h"
#include "../../events/handler/MouseEventHandler.h"
#include "../../ecs/Ecs.h"
#include "../../camera/Camera.h"

class MouseWorldNavigatorContext
{
public:
	static inline UniqueCursorPtr navCursor = nullptr;
	static inline const MouseEventHandler* mouseEventHandler = nullptr;
	static inline Camera* camera;
	static inline bool inNavMode = false;
	static inline bool leftButtonPressed = false;
	static inline std::optional<SDL_FPoint> scrollOffset{};

private:
	MouseWorldNavigatorContext() = default;
};


class MouseWorldNavigator
{
public:
	static constexpr Dimensions<int> kScreenEdgeScrollBuffer = { 50, 50 };
	static constexpr float kScrollMaxSpeed = 750.0f;
	static constexpr float kScrollMaxDistance = 550.0f;

	static Result<Void> Init(const MouseEventHandler& mouseEvHandler, Camera& cam);

	static void Update(double deltaTime);

private:
	static void HandleFreeScrollMode(const MouseState& mouseState, double deltaTime);

	MouseWorldNavigator() = default;
};

#endif