#pragma once
#include "KeyboardInputMap.h"
#include "../InputUpdater.h"
#include <SDL_events.h>

class EventBus;

class KeyboardInputUpdater
{
public:
	KeyboardInputUpdater() : inputs_(MakeInputMap<KeyboardInputMap>()) {}
	~KeyboardInputUpdater() = default;

	void Update(float dt, const SDL_Event& ev);
	void FinalizeAndPushEvents(float dt, EventBus& bus);

	KeyboardInputMap& GetInputMap() { return inputs_; }
	const KeyboardInputMap& GetInputMap() const { return inputs_; }

private:
	class Tracker
	{
	public:
		constexpr Tracker() : updated() { timestamps.fill(0); }

		std::bitset<enum_size_v<KeyboardInputSource>> updated;
		std::array<uint32_t, enum_size_v<KeyboardInputSource>> timestamps;
	};

	KeyboardInputMap inputs_;
	Tracker tracker_;
};
