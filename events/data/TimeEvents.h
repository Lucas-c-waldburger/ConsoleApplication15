#pragma once
#include <optional>
#include "../IEventData.h"
#include "../EventConcepts.h"
#include "../../ecs/EntityT.h"

namespace events {

struct TimerFired : IEventData<TimerFired> 
{
	Entity_t owner = kInvalidEntity;
	float duration = 0.0f;
};

} // events