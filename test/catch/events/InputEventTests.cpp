#include "../CatchUtils.h"
#include "../../../core/Algorithms.h"
#include "../../../events/EventBus2.h"
#include "../../../events/data/KeyboardEvents.h"
#include "../../../inputs/keyboard/KeyboardInputMap.h"

namespace ev = events;

TEST_CASE("KeyboardInput Event Test", "[events][input]")
{
	using Key = KeyboardInputSource;

	EventBus eventBus{};

	std::string keyOutput;

	auto tk = eventBus.ConnectToEvent([&keyOutput](const ev::KeyboardInput& ev) {
		if (ev.input.state == InputState::Pressed || ev.input.state == InputState::Held)
		{
			if (ev.input.value != '\0')
			{
				keyOutput += ev.input.value;
			}
		}
	});

	KeyboardInputMap keyMap = MakeInputMap<KeyboardInputMap>();
	keyMap[Key::H].state = InputState::Pressed;
	keyMap[Key::O].state = InputState::Pressed;
	keyMap[Key::L].state = InputState::Pressed;
	keyMap[Key::Y].state = InputState::Pressed;

	for (const auto& [_, inp] : keyMap)
	{
		if (inp.state == InputState::Pressed)
		{
			eventBus.PushEvent(ev::KeyboardInput{ .input = inp  });
		}
	}

	eventBus.DispatchEvents();

	CHECK(keyOutput.size() == 4);
	CHECK(core::Contains(keyOutput, 'h'));
	CHECK(core::Contains(keyOutput, 'o'));
	CHECK(core::Contains(keyOutput, 'l'));
	CHECK(core::Contains(keyOutput, 'y'));
}