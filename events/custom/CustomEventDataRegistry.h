#pragma once
#include "data/EntityCollision.h"
#include "data/GameControllerEvents.h"
#include "data/EntityActions.h"

#define CUSTOM_EVENT_DATA_REGISTRY \
	events::GameControllerConnected, \
	events::GameControllerDisconnected, \
	events::ContactCollisionBegin, \
	events::ContactCollisionEnd, \
	events::SensorCollisionBegin, \
	events::SensorCollisionEnd, \
	events::HitCollision, \
	events::EntityCreated, \
	events::EntityDestroyed, \
	events::EntityPositionChanged
