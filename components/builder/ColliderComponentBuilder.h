#pragma once
#include "../../physics/B2Body.h"
#include "ComponentBuilder.h"
#include "../ColliderComponent.h"


struct ColliderSettings
{
    float density = 1.0f;
    float friction = 0.5f;
    float restitution = 0.0f;

    struct {
        bool contact = false;
        bool sensor = false; 
        bool hit = false;
    } enableEvents;
    
    bool isSensor = false;
};

template <>
class ComponentBuilder<Collider>
{
public:
    ComponentBuilder() : settings_(), shapeParams_() {}

    ComponentBuilder& WithColliderSettings(ColliderSettings settings)
    {
        settings_ = std::move(settings);
        return *this;
    }

    ComponentBuilder& WithShapeParameters(B2ShapeParameters params)
    {
        shapeParams_ = std::move(params);
        return *this;
    }

    Collider Build(B2Body& body)
    {
        if (!body.IsValid())
        {
            return {};
        }

        B2ShapeDefinition definition;

        definition.shapeParams = std::move(shapeParams_);

        definition.shapeDef.density = settings_.density;
        definition.shapeDef.material.friction = settings_.friction;
        definition.shapeDef.material.restitution = settings_.restitution;
        definition.shapeDef.enableContactEvents = settings_.enableEvents.contact;
        definition.shapeDef.enableSensorEvents = settings_.enableEvents.sensor;
        definition.shapeDef.enableHitEvents = settings_.enableEvents.hit;
        definition.shapeDef.isSensor = settings_.isSensor;

        auto result = body.AddShape(definition);
        if (!result.Success())
        {
            LOG_ERROR(result.GetError());
            return {};
        }

        return Collider{ .shape = std::move(result.GetValue()) };
    }

private:
    ColliderSettings settings_;
    B2ShapeParameters shapeParams_;
};