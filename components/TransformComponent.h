#pragma once
#include "ComponentConcepts.h"
#include <SDL.h>

struct Transform
{
    SDL_FPoint position = { 0.0f, 0.0f };
    float rotation = 0.0f;
    SDL_FPoint scale = { 1.0f, 1.0f };

    constexpr bool operator==(const Transform& rhs) const
    {
        return position.x == rhs.position.x && 
               position.y == rhs.position.y &&
               rotation == rhs.rotation && 
               scale.x == rhs.scale.x && 
               scale.y == rhs.scale.y;
    }
};