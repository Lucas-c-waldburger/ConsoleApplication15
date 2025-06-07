#pragma once
#include "../../core/TypeUtils.h"
#include "../../core/Result.h"
#include <SDL.h>

enum class EventCode : Sint32
{
	Invalid = -1,
	SDL = 0,
	Input = 1,
	Engine = 2,
	Game = 3
};

static constexpr uint32_t kInvalidEventType = std::numeric_limits<uint32_t>::max();

template <typename Derived>
struct CustomEvent 
{
	template <typename...>
	friend Result<Void> RegisterEvents();

	static constexpr uint32_t GetEventType() { return eventType_; }

private:
	static inline uint32_t eventType_ = kInvalidEventType;
};

template <typename T>
concept SomeCustomEvent = std::derived_from<T, CustomEvent<T>> && requires()
{
	{ T::GetEventType() } -> std::convertible_to<uint32_t>;
};

namespace events {
struct SystemEventStart;
struct SystemEventEnd;
} // events

template <>
struct CustomEvent<events::SystemEventStart>
{
    template <typename...>
    friend Result<Void> RegisterEvents();

    static constexpr uint32_t GetEventType() { return eventType_; }

private:
    static inline uint32_t eventType_ = SDL_FIRSTEVENT;
};

template <>
struct CustomEvent<events::SystemEventEnd>
{
    template <typename...>
    friend Result<Void> RegisterEvents();

    static constexpr uint32_t GetEventType() { return eventType_; }

private:
    static inline uint32_t eventType_ = SDL_LASTEVENT - 1;
};

namespace events {
struct SystemEventStart : CustomEvent<SystemEventStart> {};
struct SystemEventEnd : CustomEvent<SystemEventEnd> {};
} // events

template <SomeCustomEvent...Ts>
struct is_system_event_types {
    using Types = std::tuple<Ts...>;
    static constexpr bool value = sizeof...(Ts) == 2 &&
        std::is_same_v<std::tuple_element_t<0, Types>, events::SystemEventStart> &&
        std::is_same_v<std::tuple_element_t<1, Types>, events::SystemEventEnd>;
};

template <typename...Ts>
Result<Void> RegisterEvents()
{
    static_assert((SomeCustomEvent<Ts> && ...));

    if constexpr (is_system_event_types<Ts...>::value)
    {
        return Void{};
    }

    if (!(SDL_WasInit(0)))
	{
		return MAKE_ERROR("SDL not initialized");
	}

    bool allUnregistered = ((Ts::eventType_ == kInvalidEventType) && ...);
    if (!allUnregistered)
    {
        return MAKE_ERROR("One or more event types already registered\n");
    }

    uint32_t startingType = SDL_RegisterEvents(sizeof...(Ts));
    if (startingType == kInvalidEventType)
    {
        return MAKE_ERROR("Max event type allotment reached\n");
    }

    uint32_t counter = startingType;

    ((Ts::eventType_ = counter++), ...);

    assert(counter == startingType + sizeof...(Ts));

    return Void{};
}





//// forward decl for concept
//template <typename T, EventCode code>
//struct CustomEventData;
//
//template <typename T>
//concept SomeCustomEvent = std::derived_from<T, CustomEventData<T, T::eventCode>>;
//
//inline EventCode GetEventCode(const SDL_Event& ev)
//{
//	if (ev.type >= SDL_FIRSTEVENT && ev.type < SDL_USEREVENT)
//	{
//		return EventCode::SDL;
//	}
//	if (ev.type >= SDL_LASTEVENT)
//	{
//		return EventCode::Invalid;
//	}
//
//	return static_cast<EventCode>(ev.user.code);
//}
//
//template <SomeCustomEvent T>
//inline constexpr EventCode GetEventCode()
//{
//	return T::eventCode;
//}
//
//template <typename Derived, EventCode code>
//struct CustomEventData
//{
//	template <SomeCustomEvent...Ts>
//	friend Result<Void> RegisterCustomEventDataTypes();
//
//	using EventData = Derived;
//	static uint32_t GetEventType() { return eventType; }
//
//	static constexpr EventCode eventCode = code;
//private:
//	static inline uint32_t eventType = kInvalidEventType;
//};
//
//// Events related to processing actual SDL input events
//template <typename T>
//using InputEventData = CustomEventData<T, EventCode::Input>;
//
//// Events related to ECS 
//template <typename T>
//using EngineEventData = CustomEventData<T, EventCode::Engine>;
//
//// Events for specific games developed with the engine
//template <typename T>
//using GameEventData = CustomEventData<T, EventCode::Game>;
//
//
//template <SomeCustomEvent...Ts>
//static Result<Void> RegisterCustomEventDataTypes()
//{
//	if (!(SDL_WasInit(0)))
//	{
//		return MAKE_ERROR("SDL not initialized");
//	}
//
//	uint32_t newEvType = SDL_RegisterEvents(sizeof...(Ts));
//	if (newEvType == kInvalidEventType)
//	{
//		return MAKE_ERROR("SDL_RegisterEvents failed, no valid event type");
//	}
//
//	uint32_t evTypeCounter = newEvType;
//	((Ts::eventType = ((Ts::eventType == kInvalidEventType) ? evTypeCounter++ : Ts::eventType)), ...);
//
//	if (evTypeCounter != newEvType + sizeof...(Ts))
//	{
//		return MAKE_ERROR("One or more events already had a valid event type assigned");
//	}
//
//	return Void{};
//}