#pragma once
#include "EntityActions.h"
#include "EntityCollision.h"
#include "GameControllerEvents.h"
#include "../../EventGroup.h"

namespace events {

using SystemEventGroup = EventGroup<SystemEventStart, SystemEventEnd>;

using EntityActionEventGroup = EventGroup<EntityCreated, EntityDestroyed, EntityPositionChanged>;

using GameControllerEventGroup = EventGroup<GameControllerConnected, GameControllerDisconnected>;

using CollisionEventGroup = EventGroup<ContactCollisionBegin, ContactCollisionEnd,
									   SensorCollisionBegin, SensorCollisionEnd, HitCollision>;


} // events