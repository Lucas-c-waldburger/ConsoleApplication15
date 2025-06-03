#pragma once
#include "../CustomEventData.h"
#include "../../../physics/B2Handle.h"
#include "../../../ecs/EntityT.h"

namespace events {

struct ContactCollisionBegin : EngineEventData<ContactCollisionBegin>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};

struct ContactCollisionEnd : EngineEventData<ContactCollisionEnd>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};

struct SensorCollisionBegin : EngineEventData<SensorCollisionBegin>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};

struct SensorCollisionEnd : EngineEventData<SensorCollisionEnd>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};

struct HitCollision : EngineEventData<HitCollision>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};

}