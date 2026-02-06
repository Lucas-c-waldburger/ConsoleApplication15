#pragma once
#include "../../sdl/SDLUtils.h"
#include "GameController.h"

template <SDLPointType T>
inline constexpr bool AxisOutsideDeadzone(T axisValue)
{
	return (std::abs(static_cast<int>(axisValue.x)) > GameController::kAxisDeadzone ||
		    std::abs(static_cast<int>(axisValue.y)) > GameController::kAxisDeadzone);
}