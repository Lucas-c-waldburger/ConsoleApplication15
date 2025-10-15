#pragma once
#include "../IEventData.h"
#include "../EventConcepts.h"
#include "InputEventConcepts.h"
#include "../../inputs/controller/GameControllerInputField.h"

namespace events {

struct GameControllerConnected : IEventData<GameControllerConnected>
{
	SDL_JoystickID joystickID = -1;
};

struct GameControllerDisconnected : IEventData<GameControllerDisconnected>
{
	SDL_JoystickID joystickID = -1;
};

struct GameControllerInput : IEventData<GameControllerInput>
{
	SDL_JoystickID joystickID = -1;
	GameControllerInputField input;
};
static_assert(SomeInputEvent<GameControllerInput>);

// GROUP
using GameControllerEventGroup = EventGroup<
	GameControllerDisconnected,
	GameControllerConnected,
	GameControllerInput 
>;

} // events