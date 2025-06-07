#include "EventBus.h"
//#include "../ecs/Ecs.h"

//void EventBus::DispatchEventsImpl()
//{
//	if (buffer_.Empty())
//	{
//		return;
//	}
//
//	auto bufferedEvTypes = buffer_.PeekEventTypes();
//
//	auto filter = [&bufferedEvTypes](const EventCallbacks& cbs) {
//		for (const auto& evType : bufferedEvTypes)
//		{
//			if (cbs.map.HasCallbacks(evType))
//			{
//				return true;
//			}
//		}
//		return false;
//	};
//
//	auto entities = ECS::GetAllEntitiesWith<EventCallbacks>(filter);
//	if (entities.empty())
//	{
//		return;
//	}
//
//	while (!buffer_.Empty())
//	{
//		ReturnSignal ret;
//
//		auto bufEv = buffer_.Pop();
//		assert(bufEv.type != SDL_POLLSENTINEL);
//
//		for (auto& entity : entities)
//		{
//			if (!entity.IsValid())
//			{
//				continue;
//			}
//
//			auto& cbsForType = entity.GetComponent<EventCallbacks>().map.GetCallbacks(bufEv.type);
//			if (cbsForType.empty())
//			{
//				continue;
//			}
//
//			auto it = cbsForType.begin();
//			while (it != cbsForType.end())
//			{
//				ret = ReturnSignal::StopObserving;
//
//				if (it->handle.IsValid() && it->onEvent)
//				{
//					ret = it->onEvent(bufEv);
//				}
//
//				if (ret == ReturnSignal::StopObserving)
//				{
//					it = cbsForType.erase(it);
//				}
//				else
//				{
//					++it;
//				}
//			}
//		}
//	}
//}
//
