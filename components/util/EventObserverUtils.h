#pragma once
#include "../EventObserverComponent.h"
#include "../../events/CustomEventData.h"
#include "../../ecs/Ecs.h"

template<CustomEventDataType T>
static ReturnSignal TryNotifyEntity(Entity& entity)
{
	if (T::GetEventType() == kInvalidEventType)
	{
		LOG_WARNING("Custom event did not have valid event type. Did you register it?");
		return ReturnSignal::Unknown;
	}
	if (!entity.HasComponent<EventObserver>())
	{
		LOG_WARNING("Entity did not have event observer component");
		return ReturnSignal::Unknown;
	}

	auto& events = entity.GetComponent<EventObserver>();

	auto it = events.eventCallbacks.find(T::GetEventType());
	if (it == events.eventCallbacks.end())
	{
		LOG_WARNING("Entity is not observing this event type");
		return ReturnSignal::Unknown;
	}

	auto& [func, status] = it->second;

	status = func(connectedEv, entity);
	if (status == ReturnSignal::StopObserving)
	{
		events.eventCallbacks.erase(GameControllerConnected::GetEventType());
		return ReturnSignal::StopObserving;
	}

	return status;
}
