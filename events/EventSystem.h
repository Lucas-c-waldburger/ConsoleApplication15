#pragma once
#include "../inputs/GameControllerEvents.h"
#include "EventBuffer.h"

class EventSystem
{
public:
	bool Poll(SDL_Event& ev);
	void DistributeEvents();

	template <typename T> T& GetHandler();
	template <> GameControllerEventHandler& GetHandler<GameControllerEventHandler>();

private:
	EventBuffer eventBuffer_;
	GameControllerEventHandler gameControllerHandler_;
};

template <>
inline GameControllerEventHandler& EventSystem::GetHandler()
{
	return gameControllerHandler_;
}
