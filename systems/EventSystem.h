#pragma once
#include "System.h"
#include "../ecs/Ecs.h"
#include "../events/EventGroup.h"
#include "../events/EventBus.h"
#include "../components/EventCallbacksComponent.h"

class EventSystem : public System
{
public:
	template <SomeEventGroup Group>
	void DispatchEvents();

	void Cleanup();

private:
	template <SomeEventGroup Group>
	static bool HasEventCallbacksForGroup(const EventCallbacks& cbs)
	{
		assert(Group::Range::IsValid());
		const uint32_t start = Group::Range::GetStart();
		const uint32_t end = Group::Range::GetEnd();

		for (uint32_t i = start; i <= end; i++)
		{
			auto it = cbs.map.find(i);

			if (it != cbs.map.end() && !it->second.empty())
			{
				return true;
			}	
		}

		return false;
	}
};

template<SomeEventGroup Group>
inline void EventSystem::DispatchEvents()
{
	auto events = EventBus::GetEvents<Group>();
	if (events.empty())
	{
		return;
	}

	auto entities = ECS::GetAllEntitiesWith<EventCallbacks>(
		&EventSystem::HasEventCallbacksForGroup
	);
	if (entities.empty())
	 
		return;
	}

	for (const auto& event : events)
	{
		for (auto& entity : entities)
		{
			if (!entity.HasComponent<EventCallbacks>())
			{
				continue;
			}

			auto& callbacks = entity.GetComponent<EventCallbacks>().map;

			auto entryIt = callbacks.find(event.type);
			if (entryIt == callbacks.end() || entryIt->second.empty())
			{
				continue;
			}

			auto callbacksIt = entryIt->second.begin();
			auto callbacksEnd = entryIt->second.end();

			while (callbacksIt != callbacksEnd)
			{
				auto ret = ReturnSignal::StopObserving;
				
				if (callbacksIt->onEvent)
				{
					ret = callbacksIt->onEvent(event);
				}
				
				if (ret == ReturnSignal::StopObserving)
				{
					callbacksIt = entryIt->second.erase(callbacksIt);
				}
				else
				{
					++callbacksIt;
				}
			}
		}
	}
}
