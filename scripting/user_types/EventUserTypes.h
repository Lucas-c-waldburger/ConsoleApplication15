#pragma once
#include "PhysicsUserTypes.h"
#include "GameControllerLuaUserTypes.h"
#include "../LuaUserType.h"
#include "../../events/data/EventDataIncludes.h"

/** @defgroup Collision @{ */
DEF_LUA_USERTYPE(CollisionData, Dependencies<B2ShapeHandle>) {
	lua.def_type("entity", &CollisionData::entity,
				 "shapeHandle", &CollisionData::shapeHandle);
}

using ContactCollisionBeginEvent = events::ContactCollisionBegin;
DEF_LUA_USERTYPE(ContactCollisionBeginEvent, Dependencies<CollisionData>) {
	lua.def_type("a", &ContactCollisionBeginEvent::a,
				 "b", &ContactCollisionBeginEvent::b);
}
using ContactCollisionEndEvent = events::ContactCollisionEnd;
DEF_LUA_USERTYPE(ContactCollisionEndEvent, Dependencies<CollisionData>) {
	lua.def_type("a", &ContactCollisionEndEvent::a,
				 "b", &ContactCollisionEndEvent::b);
}
/** @} */

using SensorCollisionBeginEvent = events::SensorCollisionBegin;
DEF_LUA_USERTYPE(SensorCollisionBeginEvent, Dependencies<CollisionData>) {
	lua.def_type("a", &SensorCollisionBeginEvent::a,
				 "b", &SensorCollisionBeginEvent::b);
}

using SensorCollisionEndEvent = events::SensorCollisionEnd;
DEF_LUA_USERTYPE(SensorCollisionEndEvent, Dependencies<CollisionData>) {
	lua.def_type("a", &SensorCollisionEndEvent::a,
				 "b", &SensorCollisionEndEvent::b);
}

using HitCollisionEvent = events::HitCollision;
DEF_LUA_USERTYPE(HitCollisionEvent, Dependencies<CollisionData>) {
	lua.def_type("a", &HitCollisionEvent::a,
				 "b", &HitCollisionEvent::b);
}

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