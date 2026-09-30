#pragma once
#include "ComponentConcepts.h"
#include "../inputs/controller/GameControllerInputMap.h"

struct GameControllerState
{
    SDL_JoystickID joystickID = -1;
    GameControllerInputMap inputs = MakeInputMap<GameControllerInputMap>();
};

