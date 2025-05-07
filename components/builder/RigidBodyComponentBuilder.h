#pragma once
#include "ComponentBuilder.h"
#include "../RigidBodyComponent.h"
#include "../../physics/B2World.h"

struct BodyParameters
{
    B2Body::Type bodyType = B2Body::Type::Static;
    SDL_FPoint position = { 0.0f, 0.0f };
};


template <>
class ComponentBuilder<RigidBody>
{
public:
    ComponentBuilder& WithBodyParameters(BodyParameters params)
    {
        bodyParams_ = std::move(params);
        return *this;
    }

    ComponentBuilder& WithBodyLimits(BodyLimits limits)
    {
        limits_ = std::move(limits);
        return *this;
    }

    RigidBody Build(B2World& world)
    {
        if (!world.IsValid())
        {
            return {};
        }

        B2BodyDefinition definition;
        definition.bodyData.type = static_cast<b2BodyType>(bodyParams_.bodyType);
        definition.bodyData.position = ToB2VecScaled(bodyParams_.position);

        auto result = world.AddBody(definition);
        if (!result.Success())
        {
            LOG_ERROR(result.GetError());
            return {};
        }

        RigidBody rigidBody;
        rigidBody.body = std::move(result.GetValue());
        rigidBody.limits = limits_;
        
        return rigidBody;
    }

private:
    BodyParameters bodyParams_;
    BodyLimits limits_;
};