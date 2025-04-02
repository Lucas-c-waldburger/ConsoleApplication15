#pragma once
#include <SDL.h>
#include <array>
#include "BaseComponent.h"
#include "../core/commonObjects.h"
#include "../inputs/GameController.h"
#include "../inputs/GameControllerInputStates.h"

//template <typename T>
//struct CodedSentinel;
//
//template <>
//struct CodedSentinel<SDL_JoystickID>
//{
//    static constexpr SDL_JoystickID kInvalid = -1;
//    static constexpr SDL_JoystickID kRequestNewConnection = -2;
//};

struct GameControllerState : public BaseComponent<GameControllerState, 9>
{
    enum State : uint8_t { None, Pressed, Released, Held };

    SDL_JoystickID joystickID = GameController::kInvalidJoystickID;
    HandedPair<AxisInputState> axisInput = {};
    std::array<ButtonInputState, SDL_CONTROLLER_BUTTON_MAX> buttonInput = {};
};
