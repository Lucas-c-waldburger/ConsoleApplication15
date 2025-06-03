#include "EventSystem.h"
#include "../events/EventBus.h"
#include "../ecs/Ecs.h"

void EventSystem::DispatchEvents()
{
	auto& buffer = EventBus::Get()->buffer_;

	if (buffer.Empty())
	{
		return;
	}

	auto bufferedEvTypes = buffer.PeekEventTypes();

	auto filter = [&bufferedEvTypes](const EventCallbacks& cbs) {
		for (const auto& evType : bufferedEvTypes)
		{
			if (cbs.map.HasCallbacks(evType))
			{
				return true;
			}
		}
		return false;
	};

	auto entities = ECS::GetAllEntitiesWith<EventCallbacks>(filter);
	if (entities.empty())
	{
		return;
	}

	while (!buffer.Empty())
	{
		auto bufEv = buffer.Pop();
		assert(bufEv.type != SDL_POLLSENTINEL);

		for (auto& entity : entities)
		{
			if (!entity.IsValid())
			{
				continue;
			}

			auto& cbsForType = entity.GetComponent<EventCallbacks>().map.GetCallbacks(bufEv.type);
			if (cbsForType.empty())
			{
				continue;
			}

			ReturnSignal ret;

			auto it = cbsForType.begin();
			while (it != cbsForType.end())
			{
				ret = ReturnSignal::StopObserving;

				if (it->handle.IsValid() && it->onEvent)
				{
					ret = it->onEvent(bufEv);
				}

				if (ret == ReturnSignal::StopObserving)
				{
					it = cbsForType.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	}
}

void EventSystem::ResetEventBus()
{
	EventBus::Get()->buffer_.Reset();
	EventBus::Get()->storage_.Clear();
}