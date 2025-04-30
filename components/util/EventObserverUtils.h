#pragma once
#include "../EventObserverComponent.h"
#include "../../events/custom/CustomEvents.h"
#include "../../ecs/Ecs.h"

template <CustomEventDataType T>
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

	auto newEv = CustomEvents::MakeNewEvent(std::forward<T>(eventData), code);
	assert(newEv.user.data1);

	for (auto& entity : entities)
	{
		auto& events = entity.GetComponent<EventObserver>();
		auto& [func, status] = events.eventCallbacks[T::GetEventType()];

		status = func(newEv, entity);
		if (status == ReturnSignal::StopObserving)
		{
			events.eventCallbacks.erase(T::GetEventType());
		}
	}

	return CustomEvents::FreeEvent<T>(newEv);
}
 