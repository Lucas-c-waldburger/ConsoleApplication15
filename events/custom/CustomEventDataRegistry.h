#pragma once
#include "data/EntityCollision.h"
#include "data/GameControllerConnected.h"
#include "data/EntityActions.h"

#define CUSTOM_EVENT_DATA_REGISTRY \
	GameControllerConnected, \
	GameControllerDisconnected, \
	EntityCollision::ContactBegin, \
	EntityCollision::ContactEnd, \
	EntityCollision::SensorBegin, \
	EntityCollision::SensorEnd, \
	EntityCollision::Hit, \
	events::ContactCollisionBegin, \
	events::ContactCollisionEnd, \
	events::SensorCollisionBegin, \
	events::SensorCollisionEnd, \
	events::HitCollision, \
	events::EntityCreated, \
	events::EntityDestroyed, \
	events::EntityPositionChanged
