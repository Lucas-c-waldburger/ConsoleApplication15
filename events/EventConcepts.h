#pragma once
#include "IEventData.h"

template <typename T>
	concept SomeEventData =					// type T...
	SomeTypeInList<T, EventDataTypeList>&&	// - was forward-declared and included in the master list
	std::derived_from<T, IEventData<T>>;    // - did inherit from the interface, and passed itself in


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

