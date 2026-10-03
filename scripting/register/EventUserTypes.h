#pragma once
#include "UserTypesCommon.h"
#include "../../events/data/EventDataIncludes.h"

/** @defgroup Collision @{ */
DEF_REGISTER_LUA_USERTYPE(CollisionData, "CollisionData",
	"entity", &CollisionData::entity,
	"handle", &CollisionData::shapeHandle);

#define DEF_REGISTER_LUA_USERTYPE_COLLISION_EVENT(eventType, eventName) \
DEF_REGISTER_LUA_USERTYPE(eventType, eventName, \
	"shapeA", &eventType::a, \
	"shapeB", &eventType::b, \
	"bodyEntityA", [](const eventType& ev) { return ev.entity<0>(); }, \
	"bodyEntityB", [](const eventType& ev) { return ev.entity<1>(); })

DEF_REGISTER_LUA_USERTYPE_COLLISION_EVENT(events::ContactCollisionBegin, "ContactCollisionBeginEvent");
DEF_REGISTER_LUA_USERTYPE_COLLISION_EVENT(events::ContactCollisionEnd, "ContactCollisionEndEvent");
DEF_REGISTER_LUA_USERTYPE_COLLISION_EVENT(events::SensorCollisionBegin, "SensorCollisionBeginEvent");
DEF_REGISTER_LUA_USERTYPE_COLLISION_EVENT(events::SensorCollisionEnd, "SensorCollisionEndEvent");
DEF_REGISTER_LUA_USERTYPE_COLLISION_EVENT(events::HitCollision, "HitCollisionEvent");

/** @} */

/** @defgroup GameController @{ */
DEF_REGISTER_LUA_USERTYPE(events::GameControllerConnected, "GameControllerConnectedEvent",
	"joystickID", &events::GameControllerConnected::joystickID);

DEF_REGISTER_LUA_USERTYPE(events::GameControllerDisconnected, "GameControllerDisconnectedEvent",
	"joystickID", &events::GameControllerDisconnected::joystickID);

DEF_REGISTER_LUA_USERTYPE(events::GameControllerInput, "GameControllerInputEvent",
	"joystickID", &events::GameControllerInput::joystickID,
	"input", &events::GameControllerInput::input);

/** @} */

/** @defgroup Timer @{ */
DEF_REGISTER_LUA_USERTYPE(events::TimerFired, "TimerFiredEvent",
	"entity", [](const events::TimerFired& ev) { return ev.entity<0>(); });

/** @} */