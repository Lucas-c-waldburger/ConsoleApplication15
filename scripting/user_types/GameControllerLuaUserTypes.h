#pragma once
#include "../LuaTypesRegistry.h"

template <> inline void RegisterLuaUserType<InputState>(sol::state& lua)
{
	if (!lua["InputState"].valid())
	{
		lua.new_enum("InputState",
			"None", InputState::None,
			"Pressed", InputState::Pressed,
			"Released", InputState::Released,
			"Held", InputState::Held
		);
	}
}

template <> inline void RegisterLuaUserType<GameControllerInputSource>(sol::state& lua)
{
	using Source = GameControllerInputSource;

	if (!lua["GameControllerInputSource"].valid())
	{
		lua.new_enum("GameControllerInputSource",
			"Invalid", Source::Invalid,
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
			"RightTrigger", Source::RightTrigger
		);
	}
}

template <> inline void RegisterLuaUserType<GameControllerInputField>(sol::state& lua)
{
	if (!lua["GameControllerInputFieldValue"].valid())
	{
		lua.new_usertype<GameControllerInputFieldValue>("GameControllerInputFieldValue",
			"trigger", &GameControllerInputFieldValue::trigger,
			"axis", &GameControllerInputFieldValue::axis);
	}
	if (!lua["GameControllerInputField"].valid())
	{
		lua.new_usertype<GameControllerInputField>("GameControllerInputField",
			"source", &GameControllerInputField::source,
			"state", &GameControllerInputField::state,
			"stateDuration", &GameControllerInputField::stateDuration,
			"value", &GameControllerInputField::value);
	}
}

template <> inline void RegisterLuaUserType<GameControllerInputMap>(sol::state& lua)
{
	if (!lua["GameControllerInputMap"].valid())
	{
		lua.new_usertype<GameControllerInputMap>("GameControllerInputMap",
			sol::meta_function::index, [](GameControllerInputMap& map, GameControllerInputSource source) -> GameControllerInputField& {
				return map[source];
			}
		);
	}
}

template <> inline void RegisterLuaUserType<GameControllerState>(sol::state& lua)
{
	if (!lua["GameControllerState"].valid())
	{
		lua.new_usertype<GameControllerState>("GameControllerState",
			"joystickID", &GameControllerState::joystickID,
			"inputs", &GameControllerState::inputs
		);
	}
}