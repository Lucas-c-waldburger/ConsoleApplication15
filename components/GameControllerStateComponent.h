#pragma once
#include <SDL.h>
#include <array>
#include "BaseComponent.h"
#include "../core/commonObjects.h"
#include "../inputs/GameController.h"
#include "../inputs/GameControllerInputStates.h"

struct GameControllerState : public BaseComponent<GameControllerState, 9>
{
    enum State : uint8_t { None, Pressed, Released, Held };

    SDL_JoystickID joystickID = GameController::kInvalidJoystickID;
    HandedPair<AxisInputState> axisInput = {};
    std::array<ButtonInputState, SDL_CONTROLLER_BUTTON_MAX> buttonInput = {};
};