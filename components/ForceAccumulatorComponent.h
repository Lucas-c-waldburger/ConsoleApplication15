#pragma once
#include "BaseComponent.h"
#include <SDL.h>
#include <vector>

struct Force
{
    SDL_FPoint vector = { 0, 0 };
    float duration = 0.0;
};

struct ForceAccumulator : BaseComponent<ForceAccumulator, 10>
{
    std::vector<Force> forces;
    float maxForce = 0.0;
};