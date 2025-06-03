#pragma once
#include "../customEventData.h"

struct GameControllerConnected : InputEventData<GameControllerConnected>
{
	SDL_JoystickID joystickID = -1;
};

struct GameControllerDisconnected : InputEventData<GameControllerDisconnected>
{
	SDL_JoystickID joystickID = -1;
};