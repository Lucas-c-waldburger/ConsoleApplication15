#pragma once
#include <vector>
#include <SDL.h>
#include "BaseComponent.h"

struct Force
{
    SDL_FPoint vector = { 0, 0 };
    float duration = 0.0;
};

struct AccumulatedForces 
{
    std::vector<Force> normed;
    float max = 0.0;
};

struct Physics : BaseComponent<Physics, 3>
{
    SDL_FPoint velocity = { 0.0f, 0.0f };
    SDL_FPoint acceleration = { 0.0f, 0.0f };
    float mass = 0.0f;
    float drag = 0.89f;
    float gravity = 9.81f;
    AccumulatedForces forces = {};
};