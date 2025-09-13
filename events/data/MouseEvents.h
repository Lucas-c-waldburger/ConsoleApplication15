#pragma once
#include "../IEventData.h"
#include "../EventConcepts.h"
#include "../../inputs/mouse/MouseInputMap.h"

namespace events {

struct MouseInput : IEventData<MouseInput>
{
	MouseInputField input;
	std::optional<MouseCursorInputValue> cursorValue;
	std::optional<MouseWheelInputValue> wheelValue;
};

}