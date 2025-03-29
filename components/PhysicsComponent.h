#pragma once
#include "ForceAccumulatorComponent.h"

struct Physics : BaseComponent<Physics, 3>
{
    SDL_FPoint velocity = { 0.0f, 0.0f };
    SDL_FPoint acceleration = { 0.0f, 0.0f };
    float mass = 0.0f;
    float drag = 0.0f;
    ForceAccumulator forceAccumulator = {};
};