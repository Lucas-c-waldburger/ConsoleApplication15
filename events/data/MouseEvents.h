#pragma once
#include "../IEventData.h"
#include "../EventConcepts.h"
#include "InputEventConcepts.h"
#include "../../inputs/mouse/MouseInputMap.h"

namespace events {

struct MouseInput : IEventData<MouseInput>
{
	MouseInputField input;
	MouseInputValues values;

	//std::optional<MouseCursorInputValue> cursorValue;
	//std::optional<MouseWheelInputValue> wheelValue;
};
static_assert(SomeInputEvent<MouseInput>);

}