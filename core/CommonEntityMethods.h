#pragma once
#include "../ecs/Ecs.h"

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

inline Entity GetRootEntity(Entity& entity)
{
	if (!entity.IsValid())
	{
		return {};
	}

	auto relations = entity.GetRelations();
	if (!relations.IsChild())
	{
		return entity;
	}

	return relations.GetParent();
}

inline Entity GetRootEntity(Entity_t id)
{
	auto entity = ECS::GetEntityByID(id);

	return GetRootEntity(entity);
}