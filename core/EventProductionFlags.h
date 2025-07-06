#pragma once
#include "../events/EventConcepts.h"
#include "../events/EventDataTypeList.h"
#include <bitset>

template <typename TList>
struct EventProductionFlags;

template <SomeEventData...Ts> // <- Any subset of event flags we care about
struct EventProductionFlags<TypeList<Ts...>>
{
	EventProductionFlags() : bitset(((1 << Ts::eventType) | ...)) {}

	template <SomeTypeInPack<Ts...> Us...> void Enable() 
	{ 
		((flags.set(Us::eventType, true), ...);
	}
	template <SomeTypeInPack<Ts...> Us...> void Disable() 
	{ 
		((flags.set(Us::eventType, false), ...);
	}
	void EnableAll() { flags.set(); }
	void DisableAll() { flags.reset(); }

	template <SomeTypeInPack<Ts...> U> bool ShouldProduceEvent() const
	{
		return flags.test(U::eventType);
	}

	std::bitset<EventDataTypeList::size> bitset;
};

template <SomeEventGroup Group>
struct EventGroupProductionFlags;

template <typename...Ts>
struct EventGroupProductionFlags<EventGroup<Ts...>> : EventProductionFlags<TypeList<Ts...>>
{
	// these will default to off, opt in
	constexpr EventGroupProductionFlags() : EventProductionFlags<Ts...>() { bitset.reset(); }
};