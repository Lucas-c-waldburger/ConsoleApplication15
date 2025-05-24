#pragma once
#include "../CustomEventData.h"
#include "../../../physics/B2Handle.h"
#include "../../../ecs/EntityT.h"

struct CollisionData
{
	Entity_t entityId = kInvalidEntity;
	Handle<B2Shape> shapeHandle;
};

class EntityCollision
{
public:
	struct ContactBegin : CustomEventData<ContactBegin> { CollisionData a, b; };
	struct ContactEnd   : CustomEventData<ContactEnd>   { CollisionData a, b; };
	struct SensorBegin  : CustomEventData<SensorBegin>  { CollisionData a, b; };
	struct SensorEnd    : CustomEventData<SensorEnd>    { CollisionData a, b; };
	struct Hit          : CustomEventData<Hit>          { CollisionData a, b; };

private:
	EntityCollision() = default;
};