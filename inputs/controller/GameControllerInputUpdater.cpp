#include "GameControllerInputUpdater.h"
#include "GameController.h"
#include "../../events/EventBus.h"
#include "../../events/data/GameControllerEvents.h"
#include <cassert>

namespace {

using Source = GameControllerInputSource;

constexpr bool IsSDLAxisX(const SDL_ControllerAxisEvent& ev)
{
	return ev.axis == SDL_CONTROLLER_AXIS_LEFTX || ev.axis == SDL_CONTROLLER_AXIS_RIGHTX;
}
constexpr bool IsSDLAxisY(const SDL_ControllerAxisEvent& ev)
{
	return ev.axis == SDL_CONTROLLER_AXIS_LEFTY || ev.axis == SDL_CONTROLLER_AXIS_RIGHTY;
}
constexpr bool IsSDLTrigger(const SDL_ControllerAxisEvent& ev)
{
	return ev.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || ev.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
}

constexpr bool IsInputSourceButton(GameControllerInputSource source)
{
	return source > Source::Invalid && source < Source::LeftStickAxis;
}
constexpr bool IsInputSourceAxis(GameControllerInputSource source)
{
	return source == Source::LeftStickAxis || source == Source::RightStickAxis;
}
constexpr bool IsInputSourceTrigger(GameControllerInputSource source)
{
	return source == Source::LeftTrigger || source == Source::RightTrigger;
}

constexpr bool InputHasValue(const GameControllerInputField& input)
{
	return (IsInputSourceAxis(input.source)) ? input.value.axis.x != 0 || input.value.axis.y != 0 :
		   (IsInputSourceTrigger(input.source)) ? input.value.trigger != 0 : true;
}

constexpr void AssignAxisValue(GameControllerInputField& input, const SDL_ControllerAxisEvent& ev)
{
	int newVal = (std::abs(ev.value) > GameController::kAxisDeadzone) ? ev.value : 0;

	if (IsSDLTrigger(ev))
	{
		input.value.trigger = newVal;
	}
	else if (IsSDLAxisX(ev))
	{
		input.value.axis.x = newVal;
	}
	else if (IsSDLAxisY(ev))
	{
		input.value.axis.y = newVal;
	}
}

constexpr InputState GetNextAxisState(InputState last, const GameControllerInputField& input) 
{
	if (InputHasValue(input)) 
	{
		return (last == InputState::None || last == InputState::Released)
			   ? InputState::Pressed
			   : InputState::Held;
	}
	else 
	{
		return (last == InputState::Pressed || last == InputState::Held)
			   ? InputState::Released
			   : InputState::None;
	}
}

} // unnamed

void GameControllerInputUpdater::Update(const SDL_Event& ev)
{
	using Source = GameControllerInputSource;

	if (ev.type == SDL_CONTROLLERAXISMOTION)
	{
		Source source = GetInputSourceFromSDLEnum(static_cast<SDL_GameControllerAxis>(ev.caxis.axis));
		assert(source != Source::Invalid);

		auto& input = inputs_[source];
		AssignAxisValue(input, ev.caxis);
		input.stateDuration = 0;

		size_t idx = static_cast<size_t>(source);
		tracker_.updated.set(idx);
		tracker_.timestamps[idx] = ev.caxis.timestamp;
	}
	else if (ev.type == SDL_CONTROLLERBUTTONDOWN || ev.type == SDL_CONTROLLERBUTTONUP)
	{
		Source source = GetInputSourceFromSDLEnum(static_cast<SDL_GameControllerButton>(ev.cbutton.button));
		assert(source != Source::Invalid);

		auto& input = inputs_[source];
		input.state = (ev.cbutton.state == SDL_PRESSED) ? InputState::Pressed : 
														  InputState::Released;

		input.stateDuration = 0;

		size_t idx = static_cast<size_t>(source);
		tracker_.updated.set(idx);
		tracker_.timestamps[idx] = ev.cbutton.timestamp;
	}
	else
	{
		LOG_ERROR("GameControllerInputUpdater::Update was passed an SDL_Event with an unexpected type");
	}

	return;
}

void GameControllerInputUpdater::FinalizeAndPushEvents(SDL_JoystickID ownerId, EventBus2& bus)
{
	using Source = GameControllerInputSource;

	for (size_t i = enum_start_v<Source>; i < enum_size_v<Source>; i++)
	{
		auto& input = inputs_[static_cast<Source>(i)];

		if (!tracker_.updated.test(i)) // not updated
		{
			input.stateDuration = SDL_GetTicks() - tracker_.timestamps[i];

			const auto lastState = input.state;

			input.state = (lastState == InputState::Pressed || lastState == InputState::Held)
				? InputState::Held
				: InputState::None;
		}
		else // was updated this frame
		{
			if (IsInputSourceButton(input.source))
			{
				// already updated state during the update loop, skip
			}
			else // axis or trigger, needs state update
			{
				const auto lastState = input.state;

				input.state = GetNextAxisState(lastState, input);
			}
		}

		if (input.state != InputState::None)
		{
			bus.PushEvent(events::GameControllerInput{ 
				.joystickID = ownerId, 
				.input = input 
			});
		}
	}

	tracker_.updated.reset();
}

//constexpr InputState GameControllerInputUpdater::AssignAxisValueAndReturnState(GameControllerInputField& input,
//																			   const SDL_ControllerAxisEvent& ev)
//{
//
//
//	InputState retState = InputState::None;
//	int newVal = (std::abs(ev.value) > GameController::kAxisDeadzone) ? ev.value : 0;
//
//	auto getState = [newVal](const int oldVal) -> InputState { 
//		return (oldVal != 0 && newVal == 0) ? InputState::Released :
//			   (oldVal != 0 && newVal != 0) ? InputState::Held :
//			   (oldVal == 0 && newVal != 0) ? InputState::Pressed : InputState::None;
//	};
//
//	if (IsAxisX(ev) || IsTrigger(ev))
//	{
//		int oldVal = input.value.x;
//		input.value.x = newVal;
//
//		retState = (wasReleased(oldVal)) ? InputState::Released : 
//
//		retState = (val != 0) ? InputState::Pressed : InputState::Released;
//	}
//	if (IsAxisY(ev) || IsTrigger(ev))
//	{
//		input.value.y = newVal;
//
//		retState = (val != 0) ? InputState::Pressed : InputState::Released;
//	}
//
//	return retState;
//}
