#pragma once
#include "../IEventData.h"
#include "../EventConcepts.h"
#include "InputEventConcepts.h"
#include "../../inputs/controller/GameControllerInputField.h"

namespace events {

template <typename Derived>
struct GameControllerEvent : IEventData<Derived>
{
	SDL_JoystickID joystickID = -1;
};

struct GameControllerConnected : GameControllerEvent<GameControllerConnected>
{
	SDL_JoystickID joystickID = -1;
};

struct GameControllerDisconnected : GameControllerEvent<GameControllerDisconnected>
{
	SDL_JoystickID joystickID = -1;
};

struct GameControllerInput : GameControllerEvent<GameControllerInput>
{
	SDL_JoystickID joystickID = -1;
	GameControllerInputField input;
};
static_assert(SomeInputEvent<GameControllerInput>);

// GROUP
using GameControllerEventGroup = EventGroup<
	GameControllerDisconnected,
	GameControllerConnected,
	GameControllerInput 
>;

} // events

template <typename T>
concept SomeGameControllerEvent = std::derived_from<T, events::GameControllerEvent<T>>;