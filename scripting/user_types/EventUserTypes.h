#pragma once
#include "PhysicsUserTypes.h"
#include "GameControllerLuaUserTypes.h"
#include "../LuaUserType.h"
#include "../../events/data/EventDataIncludes.h"

/** @defgroup Collision @{ */
DEF_LUA_USERTYPE(CollisionData, Dependencies<B2ShapeHandle>) {
	lua.def_type("entity", &CollisionData::entity,
				 "handle", &CollisionData::shapeHandle);
}

using ContactCollisionBeginEvent = events::ContactCollisionBegin;
DEF_LUA_USERTYPE(ContactCollisionBeginEvent, Dependencies<CollisionData>) {
	lua.def_type("shapeA", &ContactCollisionBeginEvent::a,
				 "shapeB", &ContactCollisionBeginEvent::b,
				 "bodyEntityA", [](const ContactCollisionBeginEvent& ev) { return ev.entity<0>(); },
				 "bodyEntityB", [](const ContactCollisionBeginEvent& ev) { return ev.entity<1>(); });
}
using ContactCollisionEndEvent = events::ContactCollisionEnd;
DEF_LUA_USERTYPE(ContactCollisionEndEvent, Dependencies<CollisionData>) {
	lua.def_type("shapeA", &ContactCollisionEndEvent::a,
				 "shapeB", &ContactCollisionEndEvent::b,
				 "bodyEntityA", [](const ContactCollisionEndEvent& ev) { return ev.entity<0>(); },
				 "bodyEntityB", [](const ContactCollisionEndEvent& ev) { return ev.entity<1>(); });
}

using SensorCollisionBeginEvent = events::SensorCollisionBegin;
DEF_LUA_USERTYPE(SensorCollisionBeginEvent, Dependencies<CollisionData>) {
	lua.def_type("shapeA", &SensorCollisionBeginEvent::a,
				 "shapeB", &SensorCollisionBeginEvent::b,
				 "bodyEntityA", [](const SensorCollisionBeginEvent& ev) { return ev.entity<0>(); },
				 "bodyEntityB", [](const SensorCollisionBeginEvent& ev) { return ev.entity<1>(); });
}

using SensorCollisionEndEvent = events::SensorCollisionEnd;
DEF_LUA_USERTYPE(SensorCollisionEndEvent, Dependencies<CollisionData>) {
	lua.def_type("shapeA", &SensorCollisionEndEvent::a,
				 "shapeB", &SensorCollisionEndEvent::b,
				 "bodyEntityA", [](const SensorCollisionEndEvent& ev) { return ev.entity<0>(); },
				 "bodyEntityB", [](const SensorCollisionEndEvent& ev) { return ev.entity<1>(); });
}

using HitCollisionEvent = events::HitCollision;
DEF_LUA_USERTYPE(HitCollisionEvent, Dependencies<CollisionData>) {
	lua.def_type("shapeA", &HitCollisionEvent::a,
				 "shapeB", &HitCollisionEvent::b,
				 "bodyEntityA", [](const HitCollisionEvent& ev) { return ev.entity<0>(); },
				 "bodyEntityB", [](const HitCollisionEvent& ev) { return ev.entity<1>(); });
}
/** @} */

/** @defgroup GameController @{ */
using GameControllerConnectedEvent = events::GameControllerConnected;
DEF_LUA_USERTYPE(GameControllerConnectedEvent) {
	lua.def_type("joystickID", &GameControllerConnectedEvent::joystickID);
}

using GameControllerDisconnectedEvent = events::GameControllerDisconnected;
DEF_LUA_USERTYPE(GameControllerDisconnectedEvent) {
	lua.def_type("joystickID", &GameControllerDisconnectedEvent::joystickID);
}

using GameControllerInputEvent = events::GameControllerInput;
DEF_LUA_USERTYPE(GameControllerInputEvent, Dependencies<GameControllerInputField>) {
	lua.def_type("joystickID", &GameControllerInputEvent::joystickID,
				 "input", &GameControllerInputEvent::input);
}
/** @} */

/** @defgroup Timer @{ */
using TimerFiredEvent = events::TimerFired;
DEF_LUA_USERTYPE(TimerFiredEvent) {
	lua.def_type("entity", [](const TimerFiredEvent& ev) { return ev.entity<0>(); });
}
/** @} */