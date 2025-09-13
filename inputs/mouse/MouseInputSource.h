#pragma once
#include <SDL_mouse.h>
#include "../../core/SizedEnum.h"

enum class MouseInputSource
{
	Invalid = -1,
	Cursor,
	LeftButton = SDL_BUTTON_LEFT,
	MiddleButton = SDL_BUTTON_MIDDLE,
	RightButton = SDL_BUTTON_RIGHT,
	X1 = SDL_BUTTON_X1,
	X2 = SDL_BUTTON_X2,
	Wheel,
	ENUM_SIZE_
};
static_assert(SomeSizedEnum<MouseInputSource>);



