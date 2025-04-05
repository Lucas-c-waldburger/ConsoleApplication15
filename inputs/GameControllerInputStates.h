#pragma once
#include <SDL.h>
#include <iostream>

struct AxisInputState
{
    SDL_FPoint value = { 0.0f, 0.0f };
    uint32_t timestamp = 0;
    uint8_t state = 0;
    uint32_t stateDuration = 0;
};

struct ButtonInputState
{
    SDL_GameControllerButton button = SDL_CONTROLLER_BUTTON_INVALID;
    uint32_t timestamp = 0;
    uint8_t state = 0;
    uint32_t stateDuration = 0;
};

std::ostream& operator<<(std::ostream& os, const AxisInputState& inp);
std::ostream& operator<<(std::ostream& os, const ButtonInputState& inp);