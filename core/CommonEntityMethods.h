#pragma once
#include "../ecs/Ecs.h"
#include "../events/EventConcepts.h"

template <SomeEventData T>
inline bool ShouldProduceEvent(const Entity& entity)
{
	return !entity.HasComponent<EventProductionFlags>() ||
			entity.GetComponent<EventProductionFlags>().flags.test(T::eventType);
}

 