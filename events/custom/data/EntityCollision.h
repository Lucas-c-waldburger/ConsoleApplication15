#pragma once
#include "../CustomEventData.h"
#include "../../../core/entity/EntityColliderBounds.h"

struct EntityCollision : CustomEventData<EntityCollision>
{
	EntityColliderBounds a;
	EntityColliderBounds b;
};