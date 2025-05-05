#pragma once
#include "../customEvents.h"

struct GameControllerConnected : CustomEventData<GameControllerConnected>
{
	SDL_JoystickID joystickID = -1;
};

struct GameControllerDisconnected : CustomEventData<GameControllerDisconnected>
{
	SDL_JoystickID joystickID = -1;
};