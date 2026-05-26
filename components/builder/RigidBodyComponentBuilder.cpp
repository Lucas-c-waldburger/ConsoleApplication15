#include "RigidBodyComponentBuilder.h"


BodyParameters BodyParameters::FromB2Body(const B2Body& body) 
{
    if (!body.IsValid())
    {
        return {};
    }

    return BodyParameters{
        .bodyType = body.GetBodyType(),
        .position = body.GetPosition(),
        .gravityScale = body.GetGravityScale(),
        .fixedRotation = body.IsFixedRotation()
    };
}

ComponentBuilder<RigidBody>& 
ComponentBuilder<RigidBody>::WithBodyParameters(BodyParameters params)
{
    bodyParams_ = std::move(params);
    return *this;
}

ComponentBuilder<RigidBody>& 
ComponentBuilder<RigidBody>::WithBodyLimits(BodyLimits limits)
{
    limits_ = std::move(limits);
    return *this;
}

RigidBody ComponentBuilder<RigidBody>::Build(B2World& world)
{
    if (!world.IsValid())
    {
        LOG_ERROR("B2World was invalid");
        return {};
    }

    B2BodyDefinition definition{};
    definition.bodyData.type = static_cast<b2BodyType>(bodyParams_.bodyType);
    definition.bodyData.position = ToB2VecScaled(bodyParams_.position);
    definition.bodyData.gravityScale = bodyParams_.gravityScale;
    definition.bodyData.fixedRotation = bodyParams_.fixedRotation;

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