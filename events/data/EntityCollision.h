#pragma once
#include "../IEventData.h"
#include "EntityEventConcept.h"
#include "../../physics/B2Handle.h"


struct CollisionData 
{
    Entity_t entity = kInvalidEntity;
    Handle<B2Shape> shapeHandle;
};

namespace events {

struct ContactCollisionBegin : IEventData<ContactCollisionBegin>,
                               EntityParticipants<2>
{
    CollisionData a;
    CollisionData b;
};

struct ContactCollisionEnd : IEventData<ContactCollisionEnd>,
                             EntityParticipants<2>
{
    CollisionData a;
    CollisionData b;
};

struct SensorCollisionBegin : IEventData<SensorCollisionBegin>,
                              EntityParticipants<2>
{
    CollisionData a;
    CollisionData b;
};

struct SensorCollisionEnd : IEventData<SensorCollisionEnd>,
                            EntityParticipants<2>
{
    CollisionData a;
    CollisionData b;
};

struct HitCollision : IEventData<HitCollision>,
                      EntityParticipants<2>
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