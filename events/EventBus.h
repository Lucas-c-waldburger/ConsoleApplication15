#pragma once
#include "EventGroup.h"
#include "custom/CustomEventDataRegistry.h"
#include "SDLEventBufferTransformer.h"
#include "EventStorageBinTemplate.h"


class EventBus
{
public:
    using EventTransformer = SDLEventBufferTransformer<128>;
    using EventDataStorageBin = EventDataStorageBinTemplate<CUSTOM_EVENT_DATA_REGISTRY>;

    ~EventBus() = default;
    EventBus(const EventBus&) = delete;
    EventBus(EventBus&&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus& operator=(EventBus&&) = delete;

    template <typename T>
    static Result<Void> PushEvent(T&& ev, Sint32 code = 0)
    {
        return Get()->PushEventImpl(std::forward<T>(ev), code);
    }

    static const EventDataStorageBin& GetStorage()
    {
        return Get()->storage_;
    }

    static void ClearStorage()
    {
        Get()->storage_.ClearAll();
    }

    static void FlushEvents()
    {
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
        Get()->storage_.ClearAll();
    }

    template <SomeEventGroup Group>
    static std::vector<SDL_Event> PeekEvents()
    {
        assert(Group::Range::IsValid());
        return Get()->buffer_.RetrieveEventRange(SDL_PEEKEVENT, Group::Range::GetStart(), Group::Range::GetEnd());
    }

    template <SomeCustomEvent...Ts>
    static std::vector<SDL_Event> PeekEvents()
    {
        return Get()->buffer_.RetrieveEventTypes(SDL_PEEKEVENT, Ts::GetEventType()...);
    }

    template <SomeEventGroup Group>
    static std::vector<SDL_Event> GetEvents()
    {
        assert(Group::Range::IsValid());
        return Get()->buffer_.RetrieveEventRange(SDL_GETEVENT, Group::Range::GetStart(), Group::Range::GetEnd());
    }

    template <SomeCustomEvent...Ts>
    static std::vector<SDL_Event> GetEvents()
    {
        return Get()->buffer_.RetrieveEventTypes(SDL_GETEVENT, Ts::GetEventType()...);
    }

private:
    EventBus() = default;

    template <SomeCustomEvent T>
    Result<Void> PushEventImpl(T&& evData, Sint32 code = 0)
    {
        assert(T::GetEventType() != kInvalidEventType && "invalid event type");

        const auto& storedData = storage_.Emplace(std::forward<T>(evData));

        SDL_Event ev;
        SDL_zero(ev);
        ev.type = T::GetEventType();
        ev.common.timestamp = SDL_GetTicks();
        ev.user.data1 = const_cast<T*>(&storedData);
        ev.user.data2 = nullptr;
        ev.user.code = code;

        int ret = SDL_PushEvent(&ev);
        if (ret < 0)
        {
            return MAKE_ERROR_FMT("SDL_PushEvent failed: {}", SDL_GetError());
        }
        if (ret == 0)
        {
            LOG_WARNING("Event was globally filtered out. Was this intentional?");
        }

        return Void{};
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

    EventDataStorageBin storage_{};
    EventTransformer buffer_{};
};






//class EventBus
//{
//public:
//	using EventBuffer = EventBuffer<100>;
//	using EventDataStorageBin = EventDataStorageBinTemplate<CUSTOM_EVENT_DATA_REGISTRY>;
//
//	~EventBus() = default;
//	EventBus(const EventBus&) = delete;
//	EventBus(EventBus&&) = delete;
//	EventBus& operator=(const EventBus&) = delete;
//	EventBus& operator=(EventBus&&) = delete;
//		 
//	template <typename T> requires type_in_list_v<T, typename EventDataStorageBin::Types>
//	static void BufferEvent(T&& ev)
//	{
//		return Get()->BufferEventImpl(std::forward<T>(ev));
//	}
//
//	static void DispatchEvents()
//	{
//		return Get()->DispatchEventsImpl();
//	}
//
//private:
//	EventBus() = default;
//
//	void DispatchEventsImpl();
//
//	template <typename T> 
//	void BufferEventImpl(T&& ev)
//	{
//		const auto& stackEv = storage_.GetEvents<T>().emplace_back(std::move(ev));
//
//		if (buffer_.Full())
//		{
//			LOG_WARNING_FMT("Event buffer was full! New event of type '{}' stored only", typeid(T).name());
//			return;
//		}
//
//		SDL_Event newEv = MakeEvent<T>(stackEv);
//		assert(newEv.user.data1);
//
//		assert(buffer_.Push(std::move(newEv)));
//	}
//
//	template <SomeCustomEvent T>
//	static SDL_Event MakeEvent(const T& evData)
//	{
//		SDL_Event ev{};
//		ev.type = T::GetEventType();
//		ev.user.timestamp = SDL_GetTicks();
//		ev.user.data1 = nullptr;
//		ev.user.data2 = nullptr;
//		ev.user.code = static_cast<Sint32>(T::eventCode);
//
//		if (ev.type == kInvalidEventType)
//		{
//			LOG_WARNING("Invalid event type for template arg, did you register it?");
//			return ev; 
//		}
//
//		ev.user.data1 = const_cast<T*>(&evData);
//
//		return ev;
//	}
//
//	static std::unique_ptr<EventBus>& Get()
//	{
//		static std::unique_ptr<EventBus> instance;
//		if (!instance)
//		{
//			instance = std::unique_ptr<EventBus>(new EventBus{});
//		}
//
//		return instance;
//	}
//
//	EventBuffer buffer_;
//	EventDataStorageBin storage_;
//};

