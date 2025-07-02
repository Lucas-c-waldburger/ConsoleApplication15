#pragma once
#include <bitset>
#include "GameControllerInputMap.h"

class GameControllerInputUpdater
{
public:
	GameControllerInputUpdater() = default;
	~GameControllerInputUpdater() = default;

	void Update(const SDL_Event& ev);
	void FinalizeAndPushEvents(SDL_JoystickID ownerId);

	GameControllerInputMap& GetInputMap() { return inputs_; }
	const GameControllerInputMap& GetInputMap() const { return inputs_; }

private:
	class Tracker
	{
	public:
		constexpr Tracker() : updated() { timestamps.fill(0); }

		std::bitset<kGameControllerInputSourceEnd + 1> updated;
		std::array<uint32_t, kGameControllerInputSourceEnd + 1> timestamps;
	};

	GameControllerInputMap inputs_;
	Tracker tracker_;
};