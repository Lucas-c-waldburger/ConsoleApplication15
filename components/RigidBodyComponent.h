#pragma once
#include <vector>
#include <SDL.h>
#include <optional>
#include "BaseComponent.h"
#include "../physics/B2Body.h"
#include "../core/ReadOnly.h"

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
        .max = { 
            std::numeric_limits<float>::max(), 
            std::numeric_limits<float>::max() 
        }
    };
    Range<float> angularVelocity = { 
        .min = 0.0f, 
        .max = std::numeric_limits<float>::max() 
    };
    SDL_FPoint maxImpulse = { 0.0f, 0.0f };
};

struct RigidBody : BaseComponent<RigidBody>
{
    ReadOnly<B2Body> body;
    BodyLimits limits;
    ForceRequests forceRequests; 
};