#pragma once
#include "../../core/TypeUtils.h"
#include "../../core/Result.h"
#include <SDL.h>

static constexpr uint32_t kInvalidEventType = static_cast<uint32_t>(-1);

// forward decl for concept
template <typename T>
struct CustomEventData;

template <typename T>
concept CustomEventDataType = std::derived_from<T, CustomEventData<T>>;

template <typename Derived>
struct CustomEventData
{
	template <CustomEventDataType...Ts>
	friend Result<Void> RegisterCustomEventDataTypes();

	using EventData = Derived;
	static uint32_t GetEventType() { return eventType; }
private:
	static inline uint32_t eventType = kInvalidEventType;
};

template <CustomEventDataType...Ts>
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