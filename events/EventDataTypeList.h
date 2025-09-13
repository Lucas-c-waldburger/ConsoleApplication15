#pragma once
#include "../core/TypeUtils.h"

namespace events {
	struct EntityCreated;
	struct EntityDestroyed;
	struct EntityPositionChanged;

	struct ContactCollisionBegin;
	struct ContactCollisionEnd;
	struct SensorCollisionBegin;
	struct SensorCollisionEnd;
	struct HitCollision;

	struct GameControllerConnected;
	struct GameControllerDisconnected;
	struct GameControllerInput;

	struct MouseInput;

	struct TimerFired;

	struct SpriteIndexChange;
	struct SpriteSeriesChange;

	struct GameLoopStepStart;
	struct GameLoopStepEnd;
	struct GameLoopStepRender;
}

using EventDataTypeList = TypeList<
	events::EntityCreated,
	events::EntityDestroyed,
	events::EntityPositionChanged,

	events::ContactCollisionBegin,
	events::ContactCollisionEnd,
	events::SensorCollisionBegin,
	events::SensorCollisionEnd,
	events::HitCollision,

	events::GameControllerConnected,
	events::GameControllerDisconnected,
	events::GameControllerInput,

	events::MouseInput,

	events::TimerFired,

	events::SpriteIndexChange,
	events::SpriteSeriesChange,

	events::GameLoopStepStart,
	events::GameLoopStepEnd,
	events::GameLoopStepRender
>;