#pragma once
#include "../inputs/GameControllerEvents.h"

class EventBuffer;

class EventDistributionSystem
{
public:
	bool Distribute(EventBuffer& buffer);

private:
	GameControllerEventHandler gameControllerHandler_;
};