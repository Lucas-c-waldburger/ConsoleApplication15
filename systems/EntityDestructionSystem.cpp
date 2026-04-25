#include "EntityDestructionSystem.h"
#include <ranges>
#include "../ecs/Ecs.h"
#include "../events/EventBus2.h"


void EntityDestructionSystem::Update(EventBus& bus)
{
	auto entities = ECS::GetAllEntitiesWith<MarkedDestroyed>();
	if (entities.empty())
	{
		return;
	}

	std::vector<events::EntityDestroyed> destroyEvents;
	destroyEvents.reserve(entities.size());

	for (auto& entity : entities)
	{
		destroyEvents.emplace_back(events::EntityDestroyed{ .entity = entity.GetID() });

		if (entity.GetRelations().IsParent())
		{
			const auto& children = entity.GetComponent<Children>().childEntityIds;

			size_t evsSize = destroyEvents.size();
			destroyEvents.resize(evsSize + children.size());

			std::transform(children.begin(), children.end(), 
				destroyEvents.begin() + evsSize,
				[](const auto id) { return events::EntityDestroyed{ .entity = id }; });
		}
	}

	std::ranges::sort(destroyEvents, [](const auto& lhs, const auto& rhs) {
		return lhs.entity < rhs.entity;
	});
	auto uniqueIt = std::ranges::unique(destroyEvents, [](const auto& lhs, const auto& rhs) {
		return lhs.entity == rhs.entity;
	});

	destroyEvents.erase(uniqueIt.begin(), uniqueIt.end());

	bus.PushEvents(std::move(destroyEvents));
}



