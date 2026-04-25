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
};
static_assert(SomeInputEvent<MouseInput>);

} // events