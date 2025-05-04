#pragma once
#include "../events/handler/GameControllerEventHandler.h"
#include "../events/handler/MouseEventHandler.h"
#include "../events/EventBuffer.h"
#include "System.h"

class EventSystem : public System
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
