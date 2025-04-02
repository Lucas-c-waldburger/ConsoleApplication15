#pragma once
#include "BaseComponent.h"
#include "../core/commonObjects.h"
#include <SDL.h>

struct Collider : public BaseComponent<Collider, 8>
{
    SDL_FPoint position = { 0.0f, 0.0f };
    Dimensions<float> dimensions = { 0.0f, 0.0f };
    
    float penetrationDepth = 0.0f;
    float normal = 0.0f;
};
