#pragma once
#include "BaseComponent.h"
#include <SDL.h>

struct Transform : BaseComponent<Transform, 2>
{
    SDL_FPoint position = { 0.0f, 0.0f };
    float rotation = 0.0f;
    SDL_FPoint scale = { 1.0f, 1.0f };
};