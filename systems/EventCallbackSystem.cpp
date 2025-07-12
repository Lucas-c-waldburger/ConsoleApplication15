#include "EventCallbackSystem.h"
#include "../events/EventStage.h"
#include "../events/EventCallbackRegistry.h"
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

GameControllerInputSource GetInputSource(const Event& event)
{
	const auto* inputEvent = EventDataCast<events::GameControllerInput>(event);
	if (!inputEvent)
	{
		return GameControllerInputSource::Invalid;
	}

	return inputEvent->input.source;
}

} // unnamed

std::pair<uint32_t, EventCallbackView>
EventCallbackSystem::CallbackRegistry::GetCallback(uint32_t eventType, std::string_view callbackName)
{
	return impl_.GetCallback(eventType, callbackName);
}

std::pair<uint32_t, EventCallbackView>
EventCallbackSystem::CallbackRegistry::GetCallback(uint32_t eventType, HashName callbackNameHash)
{
	return impl_.GetCallback(eventType, callbackNameHash);
}

bool EventCallbackSystem::CallbackRegistry::EraseCallback(uint32_t eventType, std::string_view callbackName)
{
	if (!impl_.EraseCallback(eventType, callbackName))
	{
		return false;
	}

	auto entities = ECS::GetAllEntitiesWith<EventCallbacks>(
		[eventType, callbackName](const EventCallbacks& callbacks) {
			auto it = callbacks.table.find(eventType);

			return it != callbacks.table.end() && it->second.name == callbackName;
		});

	for (auto& entity : entities)
	{
		auto& callbacks = entity.GetComponent<EventCallbacks>();

		callbacks.table[eventType].name = kInvalidHashName;
		callbacks.table[eventType].fn = nullptr;
	}
	
	return true;
}

void EventCallbackSystem::HandleControllerInputCallback(Entity& entity, const Event& event)
{
	if (!entity.HasComponent<GameControllerInputCallbacks>())
	{
		return;
	}

	auto& callbacks = entity.GetComponent<GameControllerInputCallbacks>();

	auto inputSource = GetInputSource(event);
	if (inputSource == GameControllerInputSource::Invalid)
	{
		return;
	}

	auto it = callbacks.table.find(inputSource);
	if (it == callbacks.table.end())
	{
		return;
	}

	if (it->second.name == kInvalidHashName)
	{
		callbacks.table.erase(it);

		return;
	}

	if (!it->second.fn) // name set but no view attached
	{
		auto view = registry_.GetCallback(events::GameControllerInput::eventType, it->second.name);
		if (!view.second.fn)
		{
			callbacks.table.erase(it);

			return;
		}

		it->second = view.second;
	}

	auto ret = it->second.fn(entity, event);
	if (ret == ReturnSignal::StopObserving)
	{
		callbacks.table.erase(it);
	}
}

void EventCallbackSystem::HandleEventCallback(Entity& entity, const Event& event)
{
	if (!(entity.IsValid() && entity.HasComponent<EventCallbacks>()))
	{
		return;
	}

	auto& callbacks = entity.GetComponent<EventCallbacks>();

	auto it = callbacks.table.find(event.type);
	if (it == callbacks.table.end())
	{
		return;
	}

	if (it->second.name == kInvalidHashName)
	{
		callbacks.table.erase(it);

		return;
	}
	
	if (!it->second.fn) // name set but no view attached
	{
		auto view = registry_.GetCallback(event.type, it->second.name);
		if (!view.second.fn)
		{
			callbacks.table.erase(it);

			return;
		}

		it->second = view.second;
	}

	auto ret = it->second.fn(entity, event);
	if (ret == ReturnSignal::StopObserving)
	{
		callbacks.table.erase(it);
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


