#pragma once
#include "ComponentBuilder.h"
#include "../RigidBodyComponent.h"
#include "../../physics/B2World.h"

struct BodyParameters
{
    B2Body::Type bodyType = B2Body::Type::Dynamic;
    SDL_FPoint position = { 0.0f, 0.0f };
    float gravityScale = 1.0f;
    bool fixedRotation = false;

    static BodyParameters FromB2Body(const B2Body& body);
};

template <>
class ComponentBuilder<RigidBody>
{
public:
    ComponentBuilder& WithBodyParameters(BodyParameters params);
    ComponentBuilder& WithBodyLimits(BodyLimits limits);

    RigidBody Build(B2World& world);

private:
    BodyParameters bodyParams_;
    BodyLimits limits_;
};