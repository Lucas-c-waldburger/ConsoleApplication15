#pragma once
#include "custom/CustomEventData.h"
#include "custom/CustomEventDataRegistry.h"
#include "EventBuffer.h"
#include "EventStorageBinTemplate.h"
#include "../core/Logger.h"

class EventBus
{
public:
	using EventBuffer = EventBuffer<100>;
	using EventDataStorageBin = EventDataStorageBinTemplate<CUSTOM_EVENT_DATA_REGISTRY>;

	friend class EventSystem;

	template <SomeCustomEvent T> requires type_in_list_v<T, typename EventDataStorageBin::Types>
	static void BufferEvent(T&& ev)
	{
		Get()->storage_.GetEvents<T>().push_back(std::move(ev));
	}

	template <SomeCustomEvent T> requires type_in_list_v<T, typename EventDataStorageBin::Types>
	static void StageEvents()
	{
		auto& events = Get()->storage_.GetEvents<T>();
		auto& buffer = Get()->buffer_;

		if (buffer.Full())
		{
			LOG_WARNING("Event buffer was full, could not load custom event");
			return;
		}

		for (const auto& ev : events)
		{
			SDL_Event newEv = MakeEvent<T>(ev);
			if (!newEv.user.data1)
			{
				continue;
			}

			bool success = buffer.Push(std::move(newEv));
			if (!success)
			{
				LOG_WARNING("Event buffer was full, could not load custom event");
				break;
			}
		}
	}


private:
	template <SomeCustomEvent T>
	static SDL_Event MakeEvent(const T& evData)
	{
		SDL_Event ev{};
		ev.type = T::GetEventType();
		ev.timestamp = SDL_GetTicks();
		ev.user.data1 = nullptr;
		ev.user.data2 = nullptr;
		ev.user.code = static_cast<Sint32>(T::eventCode);

		if (ev.type == kInvalidEventType)
		{
			LOG_WARNING("Invalid event type for template arg, did you register it?");
			return ev; 
		}

		ev.user.data1 = const_cast<T*>(&evData);

		return ev;
	}

	static std::unique_ptr<EventBus>& Get()
	{
		static std::unique_ptr<EventBus> instance;
		if (!instance)
		{
			instance = std::unique_ptr<EventBus>(new EventBus{});
		}

		return instance;
	}

	EventBuffer buffer_;
	EventDataStorageBin storage_;
};

