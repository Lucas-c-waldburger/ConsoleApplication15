#pragma once
#include "../customEventData.h"

namespace events {

struct GameControllerConnected : CustomEvent<GameControllerConnected>
{
	SDL_JoystickID joystickID = -1;
};

struct GameControllerDisconnected : CustomEvent<GameControllerDisconnected>
{
	SDL_JoystickID joystickID = -1;
};

} // events