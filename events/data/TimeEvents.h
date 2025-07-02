#pragma once
#include <optional>
#include "../IEventData.h"
#include "../EventConcepts.h"
#include "../../ecs/EntityT.h"

namespace events {

struct TimerFired : IEventData<TimerFired> 
{
	float duration = 0.0f;
	std::optional<Entity_t> parent;
};

} // events