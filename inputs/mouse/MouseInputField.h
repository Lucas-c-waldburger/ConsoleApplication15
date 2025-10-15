#pragma once
#include "MouseInputSource.h"
#include "../InputField.h"

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

struct MouseInputFieldValue
{
	MouseCursorInputValue cursor;
	MouseWheelInputValue wheel;
};

using MouseInputField = InputField<MouseInputSource, MouseInputFieldValue>;

//struct MouseInputField
//{
//	MouseInputSource source = MouseInputSource::Invalid;
//	InputState state = InputState::None;
//	uint32_t stateDuration = 0;
//};