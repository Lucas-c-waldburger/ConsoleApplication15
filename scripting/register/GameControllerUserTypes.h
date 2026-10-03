#pragma once
#include "UserTypesCommon.h"
#include "../../inputs/controller/GameControllerInputMap.h"
#include "../../components/GameControllerStateComponent.h"

DEF_REGISTER_LUA_ENUM(InputState, "InputState");
DEF_REGISTER_LUA_ENUM(GameControllerInputSource, "GameControllerInputSource");

DEF_REGISTER_LUA_USERTYPE(GameControllerInputFieldValue, "GameControllerInputFieldValue",
	"trigger", &GameControllerInputFieldValue::trigger,
	"axis", &GameControllerInputFieldValue::axis);

DEF_REGISTER_LUA_USERTYPE(GameControllerInputField, "GameControllerInputField",
	"source", &GameControllerInputField::source,
	"state", &GameControllerInputField::state,
	"stateDuration", &GameControllerInputField::stateDuration,
	"value", &GameControllerInputField::value);

DEF_REGISTER_LUA_USERTYPE(GameControllerState, "GameControllerState",
	"joystickID", &GameControllerState::joystickID,
	sol::meta_function::index, [](GameControllerState& gc, GameControllerInputSource src)
		-> GameControllerInputField& { return gc.inputs[src]; });