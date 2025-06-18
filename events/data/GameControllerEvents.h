#pragma once
#include "../IEventData.h"
#include "../EventConcepts.h"
#include "SDL_gamecontroller.h"

namespace events {

struct GameControllerConnected : IEventData<GameControllerConnected>
{
	SDL_JoystickID joystickID = -1;
};

struct GameControllerDisconnected : IEventData<GameControllerDisconnected>
{
	SDL_JoystickID joystickID = -1;
};

// GROUP
using GameControllerEventGroup = EventGroup<
	GameControllerConnected,
	GameControllerDisconnected
>;

} // events