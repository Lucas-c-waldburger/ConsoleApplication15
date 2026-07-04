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

inline constexpr std::string_view ToString(MouseInputSource src)
{
	switch (src)
	{
	case MouseInputSource::Invalid:		 return "Invalid";
	case MouseInputSource::Cursor: 		 return "Cursor";
	case MouseInputSource::LeftButton: 	 return "LeftButton";
	case MouseInputSource::MiddleButton: return "MiddleButton";
	case MouseInputSource::RightButton:	 return "RightButton";
	case MouseInputSource::X1: 			 return "X1";
	case MouseInputSource::X2: 			 return "X2";
	case MouseInputSource::Wheel: 		 return "Wheel";
	}

	return "<unknown>";
}

