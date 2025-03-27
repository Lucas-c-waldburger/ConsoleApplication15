#pragma once
//#include "../Core.h"
//#include "../components/BaseComponent.h"
#include "GameController.h"
//#include <SDL.h>
#include <unordered_map>







//struct AxisInputState
//{
//	SDL_FPoint value = { 0.0f, 0.0f };
//	uint32_t timestamp = 0;
//	uint8_t state = 0;
//	uint32_t stateDuration = 0;
//};
//
//struct ButtonInputState
//{
//	SDL_GameControllerButton button = SDL_CONTROLLER_BUTTON_INVALID;
//	uint32_t timestamp = 0;
//	uint8_t state = 0;
//	uint32_t stateDuration = 0;
//};
//
//struct GameControllerState : public BaseComponent<GameControllerState, 9>
//{
//	enum State : uint8_t { None, Pressed, Released, Held };
//
//	SDL_JoystickID joystickID = GameController::kInvalidJoystickID;
//	HandedPair<AxisInputState> axisInput = {};
//	std::array<ButtonInputState, SDL_CONTROLLER_BUTTON_MAX> buttonInput = {};
//};
