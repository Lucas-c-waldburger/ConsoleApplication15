#pragma once
#include "BaseComponent.h"
#include "../core/commonObjects.h"
#include <SDL.h>

struct Spatial : BaseComponent<Spatial, 1>
{
    SDL_FPoint position = { 0.0f, 0.0f };
    Dimensions<float> dimensions = { 0.0f, 0.0f };
};