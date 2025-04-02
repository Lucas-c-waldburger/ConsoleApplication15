#pragma once
#include "CustomEvents.h"


struct GameControllerConnected : CustomEventData<GameControllerConnected>
{
	SDL_JoystickID joystickID = -1;
};


#define CUSTOM_EVENT_DATA_REGISTRY GameControllerConnected
