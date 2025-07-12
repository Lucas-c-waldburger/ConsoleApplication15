#pragma once
#include "../core/EventDataBitset.h"

//template <typename TList>
//class EventProductionFlagsTemplate;
//
//template <SomeEventData...RelevantEventTs>
//class EventProductionFlagsTemplate<TypeList<RelevantEventTs...>>
//{
//public:
//	template <typename...Ts>
//	void Enable() requires (SomeTypeInPack<Ts, RelevantEventTs...> && ...)
//	{
//		((flags_.Set<Ts>(true)), ...);
//	}
//
//	template <typename...Ts>
//	void Disable() requires (SomeTypeInPack<Ts, RelevantEventTs...> && ...)
//	{
//		((flags_.Set<Ts>(false)), ...);
//	}
//
//	void EnableAll() { flags_.Set(); }
//	void DisableAll() { flags_.Reset(); }
//
//	template <typename T>
//	bool ShouldProduceEvent() const requires SomeTypeInPack<T, RelevantEventTs...>
//	{
//		return flags_.Test<T>();
//	}
//
//private:
//	EventDataBitset flags_;
//};
//
//using BaseEventProductionFlags = EventProductionFlagsTemplate<EventDataTypeList>;
//
//template <SomeEventGroup Group>
//class GroupEventProductionFlags;
//
//template <SomeEventData...Ts>
//class GroupEventProductionFlags<EventGroup<Ts...>> : public EventProductionFlagsTemplate<TypeList<Ts...>> {};

//template <typename TList>
//struct EventProductionFlags;
//
//// TODO <SomeEventData...RelevantEvent
//template <SomeEventData...RelevantEventTs> // <- limit the type of events we can access to only those we care about
//struct EventProductionFlags<TypeList<RelevantEventTs...>>
//{
//	template <typename Ts...> 
//	void Enable() requires (SomeTypeInPack<Ts, RelevantEventTs...> && ...)
//	{ 
//		((flags.set(Ts::eventType, true), ...);
//	}
//	  
//	template <typename Ts...>
//	void EnableOnly() requires (SomeTypeInPack<Ts, RelevantEventTs...> && ...)
//	{
//		flags.reset();
//		((flags.set(Ts::eventType, true), ...);
//	}
//
//	template <typename Ts...> 
//	void Disable() requires (SomeTypeInPack<Ts, RelevantEventTs...> && ...)
//	{ 
//		((flags.set(Ts::eventType, false), ...);
//	}
//
//	void EnableAll() { flags.set(); }
//	void DisableAll() { flags.reset(); }
//
//	template <typename T> 
//	bool ShouldProduceEvent() const requires SomeTypeInPack<T, RelevantEventTs...>
//	{
//		return flags.test(T::eventType);
//	}
//
//	std::bitset<EventDataTypeList::size> bitset = std::bitset<EventDataTypeList::size>{}.set();
//};
//
//template <SomeEventGroup Group>
//struct EventGroupProductionFlags;
//
//template <typename...RelaventEventTs>
//struct EventGroupProductionFlags<EventGroup<RelaventEventTs...>> : EventProductionFlags<TypeList<RelaventEventTs...>>
//{
//	// TODO: Should make default on/off customizable per Event Group (configuration)
//	// these will default to off, opt in
//	constexpr EventGroupProductionFlags() : EventProductionFlags<RelaventEventTs...>() { bitset.reset(); }
//};