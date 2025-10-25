#pragma once
#include "BaseComponent.h"
#include <SDL.h>

//struct Transform : BaseComponent<Transform, 2>
struct Transform : BaseComponent<Transform>
{
    SDL_FPoint position = { 0.0f, 0.0f };
    float rotation = 0.0f;
    SDL_FPoint scale = { 1.0f, 1.0f };

    friend constexpr bool operator==(const Transform& lhs, const Transform& rhs)
    {
        return lhs.position.x == rhs.position.x && 
               lhs.position.y == rhs.position.y &&
               lhs.rotation == rhs.rotation && 
               lhs.scale.x == rhs.scale.x && 
               lhs.scale.y == rhs.scale.y;
    }
};