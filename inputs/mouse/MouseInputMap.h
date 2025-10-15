#pragma once
#include "MouseInputField.h"
#include "../../core/SizedEnumMap.h"
#include <array>

using MouseInputMap = SizedEnumMap<MouseInputSource, MouseInputField>;

inline constexpr MouseInputMap MakeMouseInputMap()
{
	MouseInputMap inputMap{};

	for (size_t i = enum_start_v<MouseInputSource>; i < enum_size_v<MouseInputSource>; i++)
	{
		auto src = static_cast<MouseInputSource>(i);
		inputMap[src].source = src;
	}

	return inputMap;
}

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