#include "KeyboardInputUpdater.h"
#include <SDL_keyboard.h>
#include "../../events/EventBus2.h"
#include "../../events/data/KeyboardEvents.h"

void KeyboardInputUpdater::Update(float dt, const SDL_Event& ev)
{
	using Key = KeyboardInputSource;

	assert(ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP);

	const Key k = static_cast<Key>(ev.key.keysym.scancode);
	auto& input = inputs_[k];

	switch (ev.type)
	{
	case SDL_KEYDOWN:
		if (input.state == InputState::Released ||
			input.state == InputState::None)
		{
			assert(ev.key.repeat == 0);
			input.state = InputState::Pressed;
			input.stateDuration = 0.0f;
		}
		else // pressed or held
		{
			if (ev.key.repeat > 0)
			{
				input.state = InputState::Held;
				input.stateDuration += dt;
			}
			else
			{
				input.state = InputState::Pressed;
				input.stateDuration = 0.0f;
			}
		}
		break;

	case SDL_KEYUP:
		assert(input.state == InputState::Pressed ||
			   input.state == InputState::Held);

		input.state = InputState::Released;
		input.stateDuration = 0.0f;

		break;

	default:
		assert(false);
		break;
	}

	size_t idx = static_cast<size_t>(k);
	tracker_.updated.set(idx);
}

void KeyboardInputUpdater::FinalizeAndPushEvents(float dt, EventBus& bus)
{
	using Key = KeyboardInputSource;

	for (size_t i = enum_start_v<Key>; i < enum_size_v<Key>; ++i)
	{
		auto& input = inputs_[static_cast<Key>(i)];

		if (!tracker_.updated.test(i))
		{
			if (input.state == InputState::Released ||
				input.state == InputState::None)
			{
				input.stateDuration = (input.state == InputState::Released)
					? 0.0f
					: input.stateDuration + dt;

				input.state = InputState::None;
			}
			else // held but didnt receieve repeat keydown yet
			{
				input.stateDuration = (input.state == InputState::Pressed)
					? 0.0f
					: input.stateDuration + dt;

				input.state = InputState::Held;
			}
		}
		else
		{
			assert(input.state != InputState::None);

			bus.PushEvent(events::KeyboardInput{ .input = input });
		}
	}

	tracker_.updated.reset();
}

//void KeyboardInputUpdater::FinalizeAndPushEvents(float dt, EventBus& bus)
//{
//	using Key = KeyboardInputSource;
//
//	for (size_t i = enum_start_v<Key>; i < enum_size_v<Key>; ++i)
//	{
//		auto& input = inputs_[static_cast<Key>(i)]; 
//
//		if (!tracker_.updated.test(i))
//		{
//			const auto lastState = input.state;
//			assert(lastState == InputState::Released || lastState == InputState::None);
//
//			input.stateDuration = (lastState == InputState::Released)
//				? 0.0f
//				: input.stateDuration + dt;
//
//			input.state = InputState::None;
//		}
//		else
//		{
//			bus.PushEvent(events::KeyboardInput{ .input = input });
//		}
//	}
//}
//
//void KeyboardInputUpdater::UpdateAndPushEvents(float dt, EventBus& bus)
//{
//	using Key = KeyboardInputSource;
//
//	const Uint8* kb = SDL_GetKeyboardState(nullptr);
//
//	for (size_t i = enum_start_v<Key>; i < enum_size_v<Key>; ++i)
//	{
//		auto& keyInput = inputs_[i];
//		bool pressed = static_cast<bool>(kb[i]);
//
//		if (pressed)
//		{
//			if (keyInput.state == InputState::Released || 
//				keyInput.state == InputState::None)
//			{
//				keyInput.state = InputState::Pressed;
//				keyInput.stateDuration = 0.0f;
//			}
//			else // pressed or held
//			{
//				keyInput.state = InputState::Held;
//				keyInput.stateDuration += dt;
//			}
//		}
//		else
//		{
//			if (keyInput.state == InputState::Pressed || 
//				keyInput.state == InputState::Held)
//			{
//				keyInput.state = InputState::Released;
//				keyInput.stateDuration = 0.0f;
//			}
//			else // Released or None
//			{
//				keyInput.state = InputState::None;
//				keyInput.stateDuration += dt;
//			}			
//		}
//
//		if (keyInput.state != InputState::None)
//		{
//			bus.PushEvent(events::KeyboardInput{
//				.input = keyInput
//			});
//		}
//	}
//}