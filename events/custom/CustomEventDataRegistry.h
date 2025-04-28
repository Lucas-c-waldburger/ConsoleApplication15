#pragma once
#include "data/EntityCollision.h"
#include "data/GameControllerConnected.h"

#define CUSTOM_EVENT_DATA_REGISTRY \
	GameControllerConnected, \
	EntityCollision::ContactBegin, \
	EntityCollision::ContactEnd, \
	EntityCollision::SensorBegin, \
	EntityCollision::SensorEnd, \
	EntityCollision::Hit \
