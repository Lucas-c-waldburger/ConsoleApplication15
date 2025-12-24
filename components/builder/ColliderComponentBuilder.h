#pragma once
#include "../../physics/B2Body.h"
#include "../../physics/B2CollisionFilter.h"
#include "ComponentBuilder.h"
#include "../ColliderComponent.h"
#include "../../core/ReadOnly.h"


struct ColliderSettings
{
    float density = 1.0f;
    float friction = 0.5f;
    float restitution = 0.0f;

    struct EnableEvents {
        bool contact = false;
        bool sensor = false; 
        bool hit = false;
        static constexpr EnableEvents FromEventsEnabled(const B2Shape::EventsEnabled& en)
        {
            return { .contact = en.contact, .sensor = en.sensor, .hit = en.hit };
        }
    } enableEvents;
    
    bool enableCollision = true;
    bool isSensor = false;
};

template <>
class ComponentBuilder<Collider> : public HasWriteAccess<ComponentBuilder<Collider>, B2Body>
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

    ComponentBuilder& WithFilter(B2CollisionFilter filter)
    {
        filter_ = std::move(filter);
        return *this;
    }

    Collider Build(ReadOnly<B2Body>& roBody)
    {
        return Build(GetWriteAccess(roBody));
    }

    Collider Build(B2Body& body)
    {
        if (!body.IsValid())
        {
            return {};
        }

        B2ShapeDefinition definition;

        definition.shapeParams = shapeParams_;

        definition.shapeDef.density = settings_.density;
        definition.shapeDef.material.friction = settings_.friction;
        definition.shapeDef.material.restitution = settings_.restitution;
        definition.shapeDef.enableContactEvents = settings_.enableEvents.contact;
        definition.shapeDef.enableSensorEvents = settings_.enableEvents.sensor;
        definition.shapeDef.enableHitEvents = settings_.enableEvents.hit;
        definition.shapeDef.isSensor = settings_.isSensor;
        
        if (settings_.enableCollision)
        {
            definition.shapeDef.filter.categoryBits = filter_.categories;
            definition.shapeDef.filter.maskBits = filter_.categoryMask;
            definition.shapeDef.filter.groupIndex = filter_.groupIndex;
        }
        else
        {
            definition.shapeDef.filter.categoryBits = 0;
            definition.shapeDef.filter.maskBits = 0;
            definition.shapeDef.filter.groupIndex = 0;
        }

        auto result = body.AddShape(definition);
        if (!result.Success())
        {
            LOG_ERROR(result.GetError());
            return {};
        }

        Collider collider{};
        collider.shape = std::move(result.GetValue());

        return collider;
    }

private:
    ColliderSettings settings_;
    B2ShapeParameters shapeParams_;
    B2CollisionFilter filter_;
};