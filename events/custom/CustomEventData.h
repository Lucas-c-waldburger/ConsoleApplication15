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

static constexpr uint32_t kInvalidEventType = static_cast<uint32_t>(-1);

// forward decl for concept
template <typename T, EventCode code>
struct CustomEventData;

template <typename T>
concept SomeCustomEvent = std::derived_from<T, CustomEventData<T, T::eventCode>>;

inline EventCode GetEventCode(const SDL_Event& ev)
{
	if (ev.type >= SDL_FIRSTEVENT && ev.type < SDL_USEREVENT)
	{
		return EventCode::SDL;
	}
	if (ev.type >= SDL_LASTEVENT)
	{
		return EventCode::Invalid;
	}

	return static_cast<EventCode>(ev.user.code);
}

template <SomeCustomEvent T>
inline constexpr EventCode GetEventCode()
{
	return T::eventCode;
}

template <typename Derived, EventCode code>
struct CustomEventData
{
	template <SomeCustomEvent...Ts>
	friend Result<Void> RegisterCustomEventDataTypes();

	using EventData = Derived;
	static uint32_t GetEventType() { return eventType; }

	static constexpr EventCode eventCode = code;
private:
	static inline uint32_t eventType = kInvalidEventType;
};

// Events related to processing actual SDL input events
template <typename T>
using InputEventData = CustomEventData<T, EventCode::Input>;

// Events related to ECS 
template <typename T>
using EngineEventData = CustomEventData<T, EventCode::Engine>;

// Events for specific games developed with the engine
template <typename T>
using GameEventData = CustomEventData<T, EventCode::Game>;


template <SomeCustomEvent...Ts>
static Result<Void> RegisterCustomEventDataTypes()
{
	if (!(SDL_WasInit(0)))
	{
		return MAKE_ERROR("SDL not initialized");
	}

	uint32_t newEvType = SDL_RegisterEvents(sizeof...(Ts));
	if (newEvType == kInvalidEventType)
	{
		return MAKE_ERROR("SDL_RegisterEvents failed, no valid event type");
	}

	uint32_t evTypeCounter = newEvType;
	((Ts::eventType = ((Ts::eventType == kInvalidEventType) ? evTypeCounter++ : Ts::eventType)), ...);

	if (evTypeCounter != newEvType + sizeof...(Ts))
	{
		return MAKE_ERROR("One or more events already had a valid event type assigned");
	}

	return Void{};
}