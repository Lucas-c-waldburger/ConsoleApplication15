#pragma once
#include "../CustomEventData.h"
#include "../../../physics/B2Handle.h"

struct CollisionData
{
	Entity_t entityId = kInvalidEntity;
	Handle<B2Shape> shapeHandle;
};

struct EntityCollision
{
	struct ContactBegin : CustomEventData<ContactBegin> { CollisionData a, b; };
	struct ContactEnd   : CustomEventData<ContactEnd>   { CollisionData a, b; };
	struct SensorBegin  : CustomEventData<SensorBegin>  { CollisionData a, b; };
	struct SensorEnd    : CustomEventData<SensorEnd>    { CollisionData a, b; };
	struct Hit          : CustomEventData<Hit>          { CollisionData a, b; };
};