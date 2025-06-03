#pragma once
#include "../EventObserverComponent.h"
#include "../../events/custom/CustomEvents.h"
#include "../../ecs/Ecs.h"

template <SomeCustomEvent T>
inline Result<Void> PushEventNotification(T&& eventData, Sint32 code = 0)
{
	if (T::GetEventType() == kInvalidEventType)
	{
		return MAKE_ERROR("Custom event did not have valid event type. Did you register it?");
	}

	auto newEv = CustomEvents::MakeNewEvent(std::forward<T>(eventData), code);
	assert(newEv.user.data1);

	if (SDL_PushEvent(&newEv) < 0)
	{
		return MAKE_ERROR_FMT("Pushing custom event to SDL failed: '{}'", SDL_GetError());
	}

	return Void{};
}


template <SomeCustomEvent T>
inline Result<Void> SendEventNotification(T&& eventData, Sint32 code = 0)
{
	if (T::GetEventType() == kInvalidEventType)
	{
		return MAKE_ERROR("Custom event did not have valid event type. Did you register it?");
	}

	auto entities = ECS::GetAllEntitiesWith<EventObserver>([](const EventObserver& events) {
		auto it = events.eventCallbacks.find(T::GetEventType());

		return it != events.eventCallbacks.end() &&
			   it->second.func &&
			   it->second.status != ReturnSignal::Pause;
		});

	if (entities.empty())
	{
		return Void{};
	}

	auto newEv = CustomEvents::MakeNewEvent(std::forward<T>(eventData));
	assert(newEv.user.data1);

	for (auto& entity : entities)
	{
		auto& events = entity.GetComponent<EventObserver>();
		auto& [func, status] = events.eventCallbacks[T::GetEventType()];

		// TODO: should skip or erase on a null callback?
		if (!func)
		{
			continue;
		}
		if (status == ReturnSignal::Pause)
		{
			continue;
		}

		status = func(newEv, entity);
		if (status == ReturnSignal::StopObserving)
		{
			events.eventCallbacks.erase(T::GetEventType());
		}
	}

	return CustomEvents::FreeEvent<T>(newEv);
}
 