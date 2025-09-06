#include "EventCallbackSystem.h"
#include "../events/EventStage.h"
#include "../callbacks/EventCallbackRegistry.h"
#include "../core/CommonEntityMethods.h"
#include <algorithm>
#include <unordered_set>

namespace {

GameControllerInputSource GetInputSource(const Event& event)
{
	const auto* inputEvent = EventDataCast<events::GameControllerInput>(event);
	if (!inputEvent)
	{
		return GameControllerInputSource::Invalid;
	}

	return inputEvent->input.source;
}

bool EventCallbackViewValid(const EventCallback::View& view)
{
	return !view.name.empty() && view.eventType != kInvalidEventType;
}

} // unnamed

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

	auto handleIt = it->second.begin();
	while (handleIt != it->second.end())
	{
		auto viewResult = registry_.GetCallbackView(*handleIt);
		if (!viewResult.Success())
		{
			LOG_ERROR(viewResult.GetError());
			handleIt = it->second.erase(handleIt);

			continue;
		}

		auto& view = viewResult.GetValue();

		if (view.eventType != events::GameControllerInput::eventType)
		{
			LOG_ERROR("Event callback did not have event type matching GameControllerInput");
			handleIt = it->second.erase(handleIt);

			continue;
		}

		assert(view.fn);

		auto ret = view.fn(entity, event);
		if (ret == ReturnSignal::StopObserving)
		{
			handleIt = it->second.erase(handleIt);

			continue;
		}

		++handleIt;
	}

	if (it->second.empty())
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

	auto& callbacks = entity.GetComponent<EventCallbacks>().table;

	auto it = callbacks.find(event.type);
	if (it == callbacks.end())
	{
		return;
	}

	auto handleIt = it->second.begin();
	while (handleIt != it->second.end())
	{
		auto viewResult = registry_.GetCallbackView(*handleIt);
		if (!viewResult.Success())
		{
			LOG_ERROR(viewResult.GetError());
			handleIt = it->second.erase(handleIt);

			continue;
		}

		auto& view = viewResult.GetValue();

		assert(view.fn);

		auto ret = view.fn(entity, event);
		if (ret == ReturnSignal::StopObserving)
		{
			handleIt = it->second.erase(handleIt);

			continue;
		}

		++handleIt;
	}

	if (it->second.empty())
	{
		callbacks.erase(it);
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


