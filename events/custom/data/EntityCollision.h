#pragma once
#include "../CustomEventData.h"
#include "../../../physics/B2Handle.h"
#include "../../../ecs/EntityT.h"

namespace events {

struct ContactCollisionBegin : CustomEvent<ContactCollisionBegin>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};

struct ContactCollisionEnd : CustomEvent<ContactCollisionEnd>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};

struct SensorCollisionBegin : CustomEvent<SensorCollisionBegin>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};

struct SensorCollisionEnd : CustomEvent<SensorCollisionEnd>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};

struct HitCollision : CustomEvent<HitCollision>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};

} // events