#pragma once
#include "MouseInputField.h"
#include "../InputMap.h"
#include <array>

using MouseInputMap = InputMap<MouseInputSource, MouseInputField>;

inline constexpr bool MouseCursorMovement(const MouseInputMap& mouseInputMap)
{
	return mouseInputMap[MouseInputSource::Cursor].state == InputState::Pressed ||
		   mouseInputMap[MouseInputSource::Cursor].state == InputState::Held;
}

inline constexpr bool MouseWheelMovement(const MouseInputMap& mouseInputMap)
{
	return mouseInputMap[MouseInputSource::Wheel].state == InputState::Pressed ||
		   mouseInputMap[MouseInputSource::Wheel].state == InputState::Held;
}