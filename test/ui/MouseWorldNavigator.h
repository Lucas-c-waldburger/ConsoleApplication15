#pragma once
#include "DataEditDisplayUtils.h"
#include "../../events/handler/MouseEventHandler.h"
#include "../../ecs/Ecs.h"

class MouseWorldNavigatorContext
{
public:
	static inline Entity mouseEntity{};
	static inline UniqueCursorPtr navCursor = nullptr;
	static inline const MouseEventHandler* mouseEventHandler = nullptr;
	static inline bool inNavMode = false;


private:
	MouseWorldNavigatorContext() = default;
};


class MouseWorldNavigator
{
public:
	//static constexpr Dimensions<int> kScreenEdgeScrollBuffer = { 50, 50 };
	//static constexpr int kFrameScreenScrollAmount = 1;

	static Result<Void> Init(const MouseEventHandler& mouseEvHandler);

	static void Update();

private:
	MouseWorldNavigator() = default;
};