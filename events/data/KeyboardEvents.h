#pragma once
#include "../IEventData.h"
#include "../EventConcepts.h"
#include "InputEventConcepts.h"
#include "../../inputs/keyboard/KeyboardInputField.h"

namespace events {

struct KeyboardInput : IEventData<KeyboardInput>
{
	KeyboardInputField input;
};
static_assert(SomeInputEvent<KeyboardInput>);


} // events