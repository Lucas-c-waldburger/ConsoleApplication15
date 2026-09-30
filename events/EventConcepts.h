#pragma once
#include "IEventData.h"

template <typename T>
concept SomeEventData =					    // type T...
	SomeTypeInList<T, EventDataTypeList>;

template <SomeEventData T>
struct event_traits
{
	static constexpr uint32_t index = index_of_v<T, EventDataTypeList>;
};

template <typename T>
concept SomeUserEvent =
	SomeTypeInList<T, UserEventTypeList>;

template <SomeEventData...Ts>
using EventGroup = TypeList<Ts...>; // bundle certain events together 

namespace detail {
template <typename T>
struct is_event_group : std::false_type {};

template <typename...Ts>
struct is_event_group<EventGroup<Ts...>> : std::true_type {};
} // detail

template <typename T>
concept SomeEventGroup = detail::is_event_group<T>::value;
