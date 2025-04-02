#pragma once
#include "../core/TypeUtils.h"
#include "../core/Result.h"
#include <SDL.h>

static constexpr uint32_t kInvalidEventType = static_cast<uint32_t>(-1);

template <typename Derived>
struct CustomEventData
{
	template <typename T>
	friend Result<Void> RegisterCustomEventDataTypes();

	using EventData = Derived;
	static uint32_t GetEventType() { return eventType; }
private:
	static inline uint32_t eventType = kInvalidEventType;
};

// concept
namespace detail {

template <typename T>
struct is_custom_event_data_type : std::false_type {};

template <typename T>
struct is_custom_event_data_type<CustomEventData<T>> : std::true_type {};

} // detail

template <typename T>
static constexpr bool is_custom_event_data_type_v = detail::is_custom_event_data_type<T>::value;

template <typename T>
concept CustomEventDataType = std::derived_from<T, CustomEventData<T>>;


// registration
namespace detail {

template <typename T>
struct register_custom_event_data_types;

template <CustomEventDataType...Ts>
struct register_custom_event_data_types<TypeList<Ts...>> 
{
	static Result<Void> Call()
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
		((Ts::eventType = ((Ts::eventType != kInvalidEventType) ? evTypeCounter++ : Ts::eventType)), ...);

		if (newEvType != )

		return Void{};
	}
};

} // detail

template <typename T>
static Result<Void> RegisterCustomEventDataTypes()
{
	return detail::register_custom_event_data_types<T>::Call();
}