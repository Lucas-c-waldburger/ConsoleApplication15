#pragma once
#include "../LuaUserType.h"
#include "../../inputs/controller/GameControllerInputMap.h"
#include "../../components/GameControllerStateComponent.h"

DEF_LUA_USERTYPE(InputState) {
	lua.def_enum("None", InputState::None,
				 "Pressed", InputState::Pressed,
				 "Released", InputState::Released,
				 "Held", InputState::Held);
}

DEF_LUA_USERTYPE(GameControllerInputSource) {
	using Source = GameControllerInputSource;
	lua.def_enum("Invalid", Source::Invalid,
				 "A", Source::A,
				 "B", Source::B,
				 "X", Source::X,
				 "Y", Source::Y,
				 "Back", Source::Back,
				 "Guide", Source::Guide,
				 "Start", Source::Start,
				 "LeftStickButton", Source::LeftStickButton,
				 "RightStickButton", Source::RightStickButton,
				 "LeftShoulder", Source::LeftShoulder,
				 "RightShoulder", Source::RightShoulder,
				 "DPadUp", Source::DPadUp ,
				 "DPadDown", Source::DPadDown,
				 "DPadLeft", Source::DPadLeft,
				 "DPadRight", Source::DPadRight,
				 "Misc1", Source::Misc1,
				 "Paddle1", Source::Paddle1,
				 "Paddle2", Source::Paddle2,
				 "Paddle3", Source::Paddle3,
				 "Paddle4", Source::Paddle4,
				 "Touchpad", Source::TouchPad,
				 "LeftStickAxis", Source::LeftStickAxis,
				 "RightStickAxis", Source::RightStickAxis,
				 "LeftTrigger", Source::LeftTrigger,
				 "RightTrigger", Source::RightTrigger);
}

DEF_LUA_USERTYPE(GameControllerInputFieldValue, Dependencies<SDL_Point>) {
	lua.def_type("trigger", &GameControllerInputFieldValue::trigger,
				 "axis", &GameControllerInputFieldValue::axis);
}

DEF_LUA_USERTYPE(GameControllerInputField, Dependencies<GameControllerInputSource, InputState,
														GameControllerInputFieldValue>) {
	lua.def_type("source", &GameControllerInputField::source,
				 "state", &GameControllerInputField::state,
				 "stateDuration", &GameControllerInputField::stateDuration,
				 "value", &GameControllerInputField::value);
}

//DEF_LUA_USERTYPE(GameControllerInputMap, Dependencies<GameControllerInputField>) {
//	lua.def_type("at", [](GameControllerInputMap& map, GameControllerInputSource src) 
//		-> GameControllerInputField& { return map[src]; });
//}

DEF_LUA_USERTYPE(GameControllerState, Dependencies<GameControllerInputField>) {
	lua.def_type("joystickID", &GameControllerState::joystickID,
		sol::meta_function::index, [](GameControllerState& gc, GameControllerInputSource src) 
		-> GameControllerInputField& { return gc.inputs[src]; });
}