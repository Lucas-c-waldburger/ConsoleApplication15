#pragma once
#include <bitset>
#include "GameControllerInputMap.h"
#include "../../events/EventBus2.h"

class GameControllerInputUpdater
{
public:
	GameControllerInputUpdater() : inputs_(MakeGameControllerInputMap()), tracker_() {}
	~GameControllerInputUpdater() = default;

	void Update(const SDL_Event& ev);
	void FinalizeAndPushEvents(SDL_JoystickID ownerId, EventBus2& bus);

	GameControllerInputMap& GetInputMap() { return inputs_; }
	const GameControllerInputMap& GetInputMap() const { return inputs_; }

private:
	class Tracker
	{
	public:
		constexpr Tracker() : updated() { timestamps.fill(0); }

		std::bitset<enum_size_v<GameControllerInputSource>> updated;
		std::array<uint32_t, enum_size_v<GameControllerInputSource>> timestamps;
	};

	GameControllerInputMap inputs_;
	Tracker tracker_;
};