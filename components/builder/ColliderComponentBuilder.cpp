#include "ColliderComponentBuilder.h"

ColliderSettings ColliderSettings::FromB2Shape(const B2Shape& shape)
{
	if (!shape.IsValid())
	{
		return {};
	}

	return ColliderSettings{
		.density = shape.GetDensity(),
		.friction = shape.GetFriction(),
		.restitution = shape.GetRestitution(),
		.enableEvents = EnableEvents::FromEventsEnabled(shape.GetEventsEnabled()),
		.enableCollision = shape.IsCollisionEnabled(),
		.isSensor = shape.IsSensor()
	};
}