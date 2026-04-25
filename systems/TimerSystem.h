#pragma once
#include "System.h"
#include "../events/EventBus2.h"


class TimerSystem : public System
{
public:
	void Update(float delta, EventBus& bus);
};
