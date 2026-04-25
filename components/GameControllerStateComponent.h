#pragma once
#include "BaseComponent.h"
#include "../inputs/controller/GameControllerInputMap.h"

struct GameControllerState : public BaseComponent<GameControllerState>
{
    SDL_JoystickID joystickID = -1;
    GameControllerInputMap inputs = MakeInputMap<GameControllerInputMap>();
};

