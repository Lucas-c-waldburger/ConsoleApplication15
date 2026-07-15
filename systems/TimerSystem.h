#pragma once
#include "System.h"
#include "Pausable.h"
#include "../events/EventBus2.h"


class TimerSystem : public System,
					public Pausable<TimerSystem>
{
public:
	friend class Pausable<TimerSystem>;

	void Update(float delta, EventBus& bus);
};
