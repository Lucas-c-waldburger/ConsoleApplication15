#include "MouseInputUpdater.h"
#include "../../sdl/SDLUtils.h"

namespace {

constexpr bool IsInputSourceButton(MouseInputSource src)
{
	using enum MouseInputSource;

	return src == LeftButton || src == MiddleButton || 
		   src == RightButton || src == X1 || src == X2;
}

constexpr InputState 
GetNextCursorOrWheelState(InputState lastState, SDL_FPoint newCursorOrWheelValue)
{
	bool movementLastFrame =
		(lastState == InputState::Pressed || lastState == InputState::Held);

	bool movementThisFrame = (newCursorOrWheelValue != SDL_FPoint{ 0.0f, 0.0f });

	if (movementLastFrame)
	{
		return (movementThisFrame) ? InputState::Held : InputState::Released;
	}
	else
	{
		//assert(movementThisFrame); // or else we shouldn't be processing this as
								   // an input that was updated this frame
		return InputState::Pressed;
	}
}

}

void MouseInputUpdater::Update(const SDL_Event& ev)
{
	using Source = MouseInputSource;

	if (ev.type == SDL_MOUSEMOTION)
	{
		values_.cursor.absolutePos = {
			static_cast<float>(ev.motion.x),
			static_cast<float>(ev.motion.y)
		};
		values_.cursor.relativePos = {
			static_cast<float>(ev.motion.xrel),
			static_cast<float>(ev.motion.yrel)
		};

		size_t idx = static_cast<size_t>(Source::Cursor);
		tracker_.updated.set(idx);
		tracker_.timestamps[idx] = ev.motion.timestamp;
	}
	else if (ev.type == SDL_MOUSEWHEEL)
	{	
		values_.wheel.scroll = SDL_FPoint{
			ev.wheel.preciseX, ev.wheel.preciseY
		};
		values_.wheel.direction = 
			static_cast<SDL_MouseWheelDirection>(ev.wheel.direction);

		size_t idx = static_cast<size_t>(Source::Wheel);
		tracker_.updated.set(idx);
		tracker_.timestamps[idx] = ev.wheel.timestamp;
	}
	else if (ev.type == SDL_MOUSEBUTTONUP || ev.type == SDL_MOUSEBUTTONDOWN)
	{
		assert(ev.button.button < enum_size_v<Source>);

		auto btnSource = static_cast<Source>(ev.button.button);
		auto& input = inputs_[btnSource];

		input.state = (ev.type == SDL_MOUSEBUTTONDOWN) 
			? InputState::Pressed 
			: InputState::Released;

		input.stateDuration = 0;

		size_t idx = static_cast<size_t>(btnSource);
		tracker_.updated.set(idx);
		tracker_.timestamps[idx] = ev.button.timestamp;
	}
	else
	{
		LOG_ERROR("MouseInputUpdater::Update was passed an "
			"SDL_Event with an unexpected type");
	}

	return;
}

void MouseInputUpdater::FinalizeAndPushEvents(float delta, EventBus2& bus)
{
	using Source = MouseInputSource;

	for (size_t i = enum_start_v<Source>; i < enum_size_v<Source>; i++)
	{
		auto& input = inputs_[static_cast<Source>(i)];

		if (!tracker_.updated.test(i)) // not updated
		{
			input.stateDuration = SDL_GetTicks() - tracker_.timestamps[i];

			const auto lastState = input.state;

			if (IsInputSourceButton(input.source))
			{
				input.state = (lastState == InputState::Pressed || lastState == InputState::Held)
					? InputState::Held
					: InputState::None;

				continue;
			}

			// no cursor movement, mouse relative movement 0
			if (input.source == Source::Cursor)
			{
				values_.cursor.relativePos = { 0.0f, 0.0f };
			}
			// no wheel movement, scroll 0
			else if (input.source == Source::Wheel)
			{
				values_.wheel.scroll = { 0.0f, 0.0f };
			}

			input.state = (lastState == InputState::Pressed || lastState == InputState::Held)
				? InputState::Released // have to determine released manually
				: InputState::None; 
		}
		else // was updated this frame
		{
			if (IsInputSourceButton(input.source))
			{
				// already updated state during the update loop, skip state update
			}
			else
			{ 
				// cursor or wheel
				const auto lastState = input.state;

				input.state = (input.source == Source::Cursor) ?
					GetNextCursorOrWheelState(lastState, values_.cursor.relativePos) :
					GetNextCursorOrWheelState(lastState, values_.wheel.scroll);

				assert(input.state != InputState::None);

				input.stateDuration = (input.state == InputState::Held)
					? input.stateDuration + static_cast<uint32_t>(delta * 1000.0f)
					: 0;			
			}
		}

		if (input.state != InputState::None)
		{
			bus.PushEvent(events::MouseInput{
				.input = input,
				.values = values_
			});
		}
	}

	tracker_.updated.reset();
}

MouseState MouseInputUpdater::ToMouseStateComponent() const
{
	return MouseState{
		.inputs = inputs_,
		.values = values_
	};
}
