#pragma once
#include <cstdint>
#include <bitset>
#include <optional>
#include <variant>
#include "../sdl/SDLUtils.h"
#include "controller/GameController.h"
#include "InputState.h"





//inline constexpr GameControllerInputField GetGameControllerInputFromSDLEvent(const SDL_Event& ev)
//{
//	GameControllerInputField input{};
//
//	if (ev.type == SDL_CONTROLLERAXISMOTION)
//	{
//		input.source = GetInputSourceFromSDLEnum(static_cast<SDL_GameControllerAxis>(ev.caxis.axis));
//	}
//	if (ev.type == SDL_CONTROLLERBUTTONDOWN || ev.type == SDL_CONTROLLERBUTTONUP)
//	{
//		input.source = GetInputSourceFromSDLEnum(static_cast<SDL_GameControllerButton>(ev.cbutton.button));
//	}
//
//	return input;
//}

//using GameControllerInputMap2 = std::unordered_map<GameControllerInputSource, GameControllerInput>;
//
//inline GameControllerInputMap2 MakeGameControllerInputMap()
//{
//	GameControllerInputMap2 map;
//	map.reserve(kGameControllerInputSourceEnd + 1);
//
//	for (size_t i = kGameControllerInputSourceStart; i <= kGameControllerInputSourceEnd; i++)
//	{
//		map[static_cast<GameControllerInputSource>(i)];
//	}
//
//	return map;
//}



struct InputData
{
    uint32_t timestamp = 0;
    uint8_t state = 0;
    uint32_t stateDuration = 0;
};

// game controller
struct AxisInputData : public InputData
{
    SDL_FPoint value = { 0.0f, 0.0f };
};

struct ButtonInputData : public InputData
{
    SDL_GameControllerButton button = SDL_CONTROLLER_BUTTON_INVALID;
};

std::ostream& operator<<(std::ostream& os, const AxisInputData& inp);
std::ostream& operator<<(std::ostream& os, const ButtonInputData& inp);

// mouse
struct MouseButtonInputData : public InputData
{
    SDL_MouseButton button = SDL_MOUSE_BUTTON_INVALID;
};
struct MousePositionInputData : public InputData
{
    SDL_Point value = { 0, 0 };
};

template <typename T>
concept SomeDerivedInputData = std::derived_from<T, InputData>;
