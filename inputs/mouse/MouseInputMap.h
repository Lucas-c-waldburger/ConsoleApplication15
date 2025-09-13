#pragma once
#include "MouseInputSource.h"
#include "../InputState.h"
#include "../../core/SizedEnumMap.h"
#include <array>

struct MouseCursorInputValue
{
	struct {
		SDL_FPoint absolute = { 0.0f, 0.0f };
		SDL_FPoint relative = { 0.0f, 0.0f };
	} position;
};

struct MouseWheelInputValue
{
	SDL_FPoint scroll = { 0.0f, 0.0f };
	SDL_MouseWheelDirection direction = SDL_MOUSEWHEEL_NORMAL;
};

struct MouseInputField
{
	MouseInputSource source = MouseInputSource::Invalid;
	InputState state = InputState::None;
	uint32_t stateDuration = 0;
};

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