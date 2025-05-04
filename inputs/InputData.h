#pragma once
#include <cstdint>
#include "../sdl/SDLUtils.h"

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
