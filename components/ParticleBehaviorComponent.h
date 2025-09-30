#pragma once
#include "BaseComponent.h"
#include "../core/EvaluationProperty.h"
#include "../render/TextureMods.h"
#include "../sdl/SDLUtils.h"

struct ParticleBehavior : BaseComponent<ParticleBehavior>
{
    float lifetime = 0.0f;
    SDL_FPoint offset = { 0.0f, 0.0f };

    EvaluationProperty<SDL_FPoint> position;
    EvaluationProperty<RGB> color;
    EvaluationProperty<int> alpha;
    EvaluationProperty<SDL_FPoint> scale;
    EvaluationProperty<float> rotation;
};