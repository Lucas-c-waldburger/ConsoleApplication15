#include "EventCallbackSystem.h"
#include "../ecs/Ecs.h"
#include "../events/EventStage.h"
#include "../events/EventCallbackRegistry.h"
#include "../core/Monitoring.h"
#include <algorithm>
#include <unordered_set>

namespace {

std::vector<uint32_t> ExtractEventTypes(EventSpan events)
{
	std::unordered_set<uint32_t> eventTypes;

	std::transform(events.begin(), events.end(), std::inserter(eventTypes, eventTypes.end()),
		[](const auto& event) { return event.type; });

	return { eventTypes.begin(), eventTypes.end() };
}

//auto eventTypes = ExtractEventTypes(events);
//
//auto entities = ECS::GetAllEntitiesWith<EventCallbacks>(
//	[&eventTypes](const EventCallbacks& callbacks) {
//		for (auto eventType : eventTypes)
//		{
//			if (callbacks.table.contains(eventType))
//			{
//				return true;
//			}
//		}
//		return false;
//	});

} // unnamed

void EventCallbackSystem::EntityDestroyed(Entity_t entityId)
{
	auto entity = ECS::GetEntityByID(entityId);
	if (!(entity.IsValid() && entity.HasComponent<EventCallbacks>()))
	{
		return;
	}

	auto& callbackTable = entity.GetComponent<EventCallbacks>().table;
	for (auto& [_, key] : callbackTable)
	{
		// if the entity being destroyed 'owns' the callback
		if (key.uniqueOwner.has_value() && *key.uniqueOwner == entityId)
		{
			callbackRegistry_.RemoveCallback(key);
		}
	}
}

void EventCallbackSystem::Dispatch(EventSpan events)
{
	if (events.empty())
	{
		return;
	}

	auto entities = ECS::GetAllEntitiesWith<EventCallbacks>();
	if (entities.empty())
	{
		return;
	}
	
	for (const Event& event : events)
	{
		for (auto& entity : entities)
		{
			// double-check that an earlier callback didn't invalidate this entity
			if (!(entity.IsValid() && entity.HasComponent<EventCallbacks>()))
			{
				continue;
			}

			auto& callbacks = entity.GetComponent<EventCallbacks>().table;

			auto it = callbacks.find(event.type);
			if (it == callbacks.end())
			{
				continue;
			}

			const auto& key = it->second;

			auto callbackView = callbackRegistry_.GetCallback(key);
			if (!callbackView)
			{
				callbacks.erase(key.eventType);
			}

			auto ret = callbackView(event, entity.GetID());
			if (ret == ReturnSignal::StopObserving)
			{
				if (key.uniqueOwner.has_value() && *key.uniqueOwner == entity.GetID())
				{
					callbackRegistry_.RemoveCallback(key);
				}

				callbacks.erase(it);
			}
		}
	}
}