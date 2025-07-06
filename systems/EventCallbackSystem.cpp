#include "EventCallbackSystem.h"
#include "../ecs/Ecs.h"
#include "../events/EventStage.h"
#include "../events/EventCallbackRegistry.h"
#include "../core/Monitoring.h"
#include "../components/GameControllerInputCallbacksComponent.h"
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

} // unnamed

void EventCallbackSystem::EntityDestroyed(Entity_t entityId)
{
	callbackRegistry_.RemoveCallbacksWithOwner(entityId);
}

void EventCallbackSystem::HandleControllerInputCallback(Entity& entity, const Event& event)
{
	if (!(entity.IsValid() && entity.HasComponent<GameControllerInputCallbacks>()))
	{
		return;
	}

	auto& inputCallbacks = entity.GetComponent<GameControllerInputCallbacks>().table;

	const auto* castEvent = EventDataCast<events::GameControllerInput>(event);
	if (!castEvent)
	{
		return;
	}

	auto it = inputCallbacks.find(castEvent->input.source);
	if (it == inputCallbacks.end())
	{
		return;
	}

	const auto& handle = it->second;

	auto callbackView = callbackRegistry_.GetCallbackView(handle);
	if (!callbackView)
	{
		inputCallbacks.erase(castEvent->input.source);

		return;
	}

	if (callbackView(entity.GetID(), event) == ReturnSignal::StopObserving)
	{
		//if (key.uniqueOwner.has_value() && *key.uniqueOwner == entity.GetID())
		//{
		//	callbackRegistry_.RemoveCallback(key);
		//}

		inputCallbacks.erase(castEvent->input.source);
	}
}

void EventCallbackSystem::HandleEventCallback(Entity& entity, const Event& event)
{
	if (!(entity.IsValid() && entity.HasComponent<EventCallbacks>()))
	{
		return;
	}

	auto& eventCallbacks = entity.GetComponent<EventCallbacks>().table;

	auto it = eventCallbacks.find(event.type);
	if (it == eventCallbacks.end())
	{
		return;
	}

	const auto& handle = it->second;

	auto callbackView = callbackRegistry_.GetCallbackView(handle);
	if (!callbackView)
	{
		eventCallbacks.erase(event.type);
	}

	auto ret = callbackView(entity.GetID(), event);
	if (ret == ReturnSignal::StopObserving)
	{
		//if (key.uniqueOwner.has_value() && *key.uniqueOwner == entity.GetID())
		//{
		//	callbackRegistry_.RemoveCallback(key);
		//}

		eventCallbacks.erase(it);
	}
}

void EventCallbackSystem::Dispatch(EventSpan events)
{
	if (events.empty())
	{
		return;
	}

	auto entities = ECS::GetAllEntitiesWithAny<EventCallbacks, GameControllerInputCallbacks>();
	if (entities.empty())
	{
		return;
	}
	
	for (const Event& event : events)
	{
		for (auto& entity : entities)
		{
			if (event.type == events::GameControllerInput::eventType)
			{
				HandleControllerInputCallback(entity, event);
			}

			HandleEventCallback(entity, event);
		}
	}
}