#pragma once
#include "../inputs/GameControllerEvents.h"
#include "EventBuffer.h"

class EventSystem
{
public:
	bool Poll(SDL_Event& ev);
	void DistributeEvents();

private:
	EventBuffer eventBuffer_;
	GameControllerEventHandler gameControllerHandler_;
};