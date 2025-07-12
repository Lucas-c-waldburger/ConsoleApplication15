#pragma once
#include "../ecs/Ecs.h"
#include "../components/EventProductionFlagsComponent.h"
#include "../components/NeedsUpdateComponent.h"

template <SomeEventData T>
inline bool EntityShouldProduceEvent(const Entity& entity)
{
	return !entity.HasComponent<EventProductionFlags>() ||
		    entity.GetComponent<EventProductionFlags>().flags.test(T::eventType);

}

template <SomeComponent T>
inline constexpr bool EntityNeedsComponentUpdate(const Entity& entity)
{
	return entity.HasComponent<NeedsUpdate>() &&
		  (entity.GetComponent<NeedsUpdate>().components & T::componentBit);
}