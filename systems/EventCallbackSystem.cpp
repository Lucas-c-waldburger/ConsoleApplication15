#include "EventCallbackSystem.h"
#include "../events/EventStage.h"
#include "../events/EventCallbackRegistry.h"
#include "../components/GameControllerInputCallbacksComponent.h"
#include "../core/CommonEntityMethods.h"
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

bool IsCallbackViewValid(const std::pair<uint32_t, EventCallbackView>& view)
{
	return view.first != kInvalidEventType && view.second.name != kInvalidHashName && 
		   view.second.fn != nullptr;
}

} // unnamed

std::pair<uint32_t, EventCallbackView> 
EventCallbackSystem::CallbackRegistry::RegisterCallback(EventCallbackFulfillmentRequest&& request)
{
	return impl_.RegisterCallback(std::move(request));
}

std::pair<uint32_t, EventCallbackView> 
EventCallbackSystem::CallbackRegistry::RegisterOrRetrieveCallback(EventCallbackFulfillmentRequest&& request)
{
	return impl_.RegisterOrRetrieveCallback(std::move(request));
}

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

	auto entities = ECS::GetAllEntitiesWith<EventCallbacks>();

	for (auto& entity : entities)
	{
		auto& callbacks = entity.GetComponent<EventCallbacks>();

		auto it = callbacks.table.find(eventType);
		if (it == callbacks.table.end() || it->second.name != callbackName)
		{
			continue;
		}

		it->second.name = kInvalidHashName;
		it->second.fn = nullptr;
	}
	
	return true;
}

void EventCallbackSystem::HandleEventCallbackFulfillmentRequests()
{
	auto entities = ECS::GetAllEntitiesWith<EventCallbackFulfillmentRequest>();

	for (auto& entity : entities)
	{
		auto& request = entity.GetComponent<EventCallbackFulfillmentRequest>();

		// validate request
		if (request.callbackName == kInvalidHashName)
		{
			LOG_WARNING("Event callback request did not have a valid callback name");
			continue;
		}

		auto eventType = request.eventType;
		if (eventType == kInvalidEventType)
		{
			LOG_WARNING("Event callback request did not have a valid event type");
			continue;
		}

		auto inputSource = request.inputSource;
		if (inputSource.has_value() && eventType != events::GameControllerInput::eventType)
		{
			LOG_WARNING("Input source in request had value, but event type was not GameControllerInput");
			continue;
		}

		auto view = registry_.RegisterCallback(std::move(request));

		if (IsCallbackViewValid(view))
		{
			auto root = GetRootEntity(entity);
			assert(root.IsValid());

			// if optional input source has value, just looking to put it into the input callbacks
			if (inputSource.has_value())
			{
				if (!root.HasComponent<GameControllerInputCallbacks>())
				{
					LOG_WARNING("Input source on request specified, but root entity did "
						"not have GameControllerInputCallbacks component");
				}
				else
				{
					auto& inputCallbacks = root.GetComponent<GameControllerInputCallbacks>();

					inputCallbacks.table[*inputSource] = view.second;
				}

				continue;
			}

			if (!root.HasComponent<EventCallbacks>())
			{
				LOG_WARNING("Root entity had not made a request for this event callback");
			}
			else
			{
				auto& eventCallbacks = root.GetComponent<EventCallbacks>();

				eventCallbacks.table.insert(std::move(view));
			}
		}
	}
}

void EventCallbackSystem::HandleControllerInputCallback(Entity& entity, const Event& event)
{
	if (!(entity.IsValid() && entity.HasComponent<GameControllerInputCallbacks>()))
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
		return;
	}

	assert(it->second.fn); // should have been fulfilled already

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
		return;
	}
	
	//if (!it->second.fn) // name set but no view attached
	//{
	//	auto view = registry_.GetCallback(event.type, it->second.name);
	//	if (!view.second.fn)
	//	{
	//		return;
	//	}

	//	it->second = view.second;
	//}

	auto ret = it->second.fn(entity, event);
	if (ret == ReturnSignal::StopObserving)
	{
		callbacks.table.erase(it);
	}
}

void EventCallbackSystem::Dispatch(EventSpan events)
{
	HandleEventCallbackFulfillmentRequests();

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


