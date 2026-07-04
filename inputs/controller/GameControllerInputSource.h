#pragma once
#include <SDL.h>
#include "../../core/SizedEnum.h"

enum class GameControllerInputSource
{
	Invalid = SDL_CONTROLLER_BUTTON_INVALID,
	A = SDL_CONTROLLER_BUTTON_A,
	B = SDL_CONTROLLER_BUTTON_B,
	X = SDL_CONTROLLER_BUTTON_X,
	Y = SDL_CONTROLLER_BUTTON_Y,
	Back = SDL_CONTROLLER_BUTTON_BACK,
	Guide = SDL_CONTROLLER_BUTTON_GUIDE,
	Start = SDL_CONTROLLER_BUTTON_START,
	LeftStickButton = SDL_CONTROLLER_BUTTON_LEFTSTICK,
	RightStickButton = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
	LeftShoulder = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
	RightShoulder = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
	DPadUp = SDL_CONTROLLER_BUTTON_DPAD_UP,
	DPadDown = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
	DPadLeft = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
	DPadRight = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
	Misc1 = SDL_CONTROLLER_BUTTON_MISC1,
	Paddle1 = SDL_CONTROLLER_BUTTON_PADDLE1,
	Paddle2 = SDL_CONTROLLER_BUTTON_PADDLE2,
	Paddle3 = SDL_CONTROLLER_BUTTON_PADDLE3,
	Paddle4 = SDL_CONTROLLER_BUTTON_PADDLE4,
	TouchPad = SDL_CONTROLLER_BUTTON_TOUCHPAD,
	LeftStickAxis = SDL_CONTROLLER_BUTTON_MAX,
	RightStickAxis,
	LeftTrigger,
	RightTrigger,
	ENUM_SIZE_
};
static_assert(SomeSizedEnum<GameControllerInputSource>);

static constexpr GameControllerInputSource kInvalidGameControllerInputSource = 
	GameControllerInputSource::ENUM_SIZE_;

template <typename T>
concept SomeSDLGameControllerInputEnum = std::same_as<T, SDL_GameControllerAxis> || 
										 std::same_as<T, SDL_GameControllerButton>;

template <SomeSDLGameControllerInputEnum T>
inline constexpr GameControllerInputSource GetInputSourceFromSDLEnum(T sdlEnum)
{
	using Source = GameControllerInputSource;

	if constexpr (std::same_as<T, SDL_GameControllerAxis>)
	{
		switch (sdlEnum)
		{
		case SDL_CONTROLLER_AXIS_LEFTX: case SDL_CONTROLLER_AXIS_LEFTY:
			return Source::LeftStickAxis;

		case SDL_CONTROLLER_AXIS_RIGHTX: case SDL_CONTROLLER_AXIS_RIGHTY:
			return Source::RightStickAxis;

		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			return Source::LeftTrigger;

		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			return Source::RightTrigger;

		default:
			return Source::Invalid;
		}
	}
	else if constexpr (std::same_as<T, SDL_GameControllerButton>)
	{
		return static_cast<Source>(sdlEnum);
	}
	else
	{
		return Source::Invalid;
	}
}

inline constexpr std::strong_ordering operator<=>(GameControllerInputSource lhs, GameControllerInputSource rhs)
{
	using Underlying = std::underlying_type_t<GameControllerInputSource>;

	return static_cast<Underlying>(lhs) <=> static_cast<Underlying>(rhs);
}

inline constexpr std::string_view ToString(GameControllerInputSource src)
{
	switch (src)
	{
	case GameControllerInputSource::Invalid:		  return "Invalid";
	case GameControllerInputSource::A:				  return "A";
	case GameControllerInputSource::B:				  return "B";
	case GameControllerInputSource::X:				  return "X";
	case GameControllerInputSource::Y:				  return "Y";
	case GameControllerInputSource::Back:			  return "Back";
	case GameControllerInputSource::Guide:			  return "Guide";
	case GameControllerInputSource::Start:			  return "Start";
	case GameControllerInputSource::LeftStickButton:  return "LeftStickButton";
	case GameControllerInputSource::RightStickButton: return "RightStickButton";
	case GameControllerInputSource::LeftShoulder:	  return "LeftShoulder";
	case GameControllerInputSource::RightShoulder:	  return "RightShoulder";
	case GameControllerInputSource::DPadUp:			  return "DPadUp";
	case GameControllerInputSource::DPadDown: 		  return "DPadDown";
	case GameControllerInputSource::DPadLeft: 		  return "DPadLeft";
	case GameControllerInputSource::DPadRight:		  return "DPadRight";
	case GameControllerInputSource::Misc1:			  return "Misc1";
	case GameControllerInputSource::Paddle1:		  return "Paddle1";
	case GameControllerInputSource::Paddle2:		  return "Paddle2";
	case GameControllerInputSource::Paddle3:		  return "Paddle3";
	case GameControllerInputSource::Paddle4:		  return "Paddle4";
	case GameControllerInputSource::TouchPad:		  return "TouchPad";
	case GameControllerInputSource::LeftStickAxis:	  return "LeftStickAxis";
	case GameControllerInputSource::RightStickAxis:	  return "RightStickAxis";
	case GameControllerInputSource::LeftTrigger:	  return "LeftTrigger";
	case GameControllerInputSource::RightTrigger:	  return "RightTrigger";
	}

	return "<unknown>";
}
