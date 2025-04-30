#pragma once
#include <vector>
#include <SDL.h>
#include <optional>
#include "BaseComponent.h"
#include "../physics/B2Body.h"

static constexpr float kDrag = 0.89f;
static constexpr float kGravity = 9.81f;

struct Force
{
    SDL_FPoint value = { 0.0f, 0.0f };
    std::optional<SDL_FPoint> worldPoint;
};

struct ForceRequests
{
    std::vector<Force> forces;
    std::vector<Force> impulses;
};

struct BodyLimits
{
    Range<SDL_FPoint> linearVelocity = {
        .min = { 0.0f, 0.0f },
        .max = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() }
    };
    Range<float> angularVelocity = { 
        .min = 0.0f, 
        .max = std::numeric_limits<float>::max() 
    };
};

struct RigidBody : BaseComponent<RigidBody, 3>
{
    Handle<B2Body> bodyHandle;
    BodyLimits limits;
    ForceRequests forceRequests; 
};