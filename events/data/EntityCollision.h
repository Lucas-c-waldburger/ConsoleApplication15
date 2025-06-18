#pragma once
#include "../IEventData.h"
#include "../EventConcepts.h"
#include "../../ecs/EntityT.h"
#include "../../physics/B2Handle.h"


struct CollisionData 
{
    Entity_t entity = kInvalidEntity;
    Handle<B2Shape> shapeHandle;
};

namespace events {

struct ContactCollisionBegin : IEventData<ContactCollisionBegin>
{
    CollisionData a;
    CollisionData b;
};

struct ContactCollisionEnd : IEventData<ContactCollisionEnd>
{
    CollisionData a;
    CollisionData b;
};

struct SensorCollisionBegin : IEventData<SensorCollisionBegin>
{
    CollisionData a;
    CollisionData b;
};

struct SensorCollisionEnd : IEventData<SensorCollisionEnd>
{
    CollisionData a;
    CollisionData b;
};

struct HitCollision : IEventData<HitCollision>
{
    CollisionData a;
    CollisionData b;
};

// GROUP
using CollisionEventGroup = EventGroup<
    ContactCollisionBegin,
    ContactCollisionEnd,
    SensorCollisionBegin,
    SensorCollisionEnd,
    HitCollision
>;


} // events