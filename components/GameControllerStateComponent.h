#pragma once
#include <SDL.h>
#include <array>
#include "BaseComponent.h"
#include "../core/commonObjects.h"
#include "../inputs/controller/GameController.h"
#include "../inputs/InputData.h"

// TODO: figure out what to do with State enum
//struct GameControllerState : public BaseComponent<GameControllerState, 9>
struct GameControllerState : public BaseComponent<GameControllerState>
{
    SDL_JoystickID joystickID = GameController::kInvalidJoystickID;
    HandedPair<AxisInputData> axisInput = {};
    std::array<ButtonInputData, SDL_CONTROLLER_BUTTON_MAX> buttonInput = {};
};
