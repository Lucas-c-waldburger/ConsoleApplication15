#pragma once
#include "MouseInputSource.h"
#include "../InputField.h"

struct MouseInputValues
{
	struct CursorValue
	{
		SDL_FPoint absolutePos = { 0.0f, 0.0f };
		SDL_FPoint relativePos = { 0.0f, 0.0f };
	};

	struct WheelValue
	{
		SDL_FPoint scroll = { 0.0f, 0.0f };
		SDL_MouseWheelDirection direction = SDL_MOUSEWHEEL_NORMAL;
	};

	CursorValue cursor;
	WheelValue wheel;
};

using MouseInputField = InputField<MouseInputSource, void>;