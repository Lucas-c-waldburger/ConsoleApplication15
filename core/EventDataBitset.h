#pragma once
#include <bitset>
#include "../events/EventConcepts.h"
#include "../events/EventDataTypeList.h"

//class EventDataBitset
//{
//public:
//	void Reset() { bitset_.reset(); }
//
//	void Set() { bitset_.set(); }
//
//	template <SomeEventData...Ts>
//	void Set(bool tf)
//	{
//		((bitset_.set(Ts::eventType, tf)), ...);
//	}
//
//	template <SomeEventData T>
//	constexpr bool Test() const { return bitset_.test(T::eventType); }
//
//private:
//	std::bitset<EventDataTypeList::size> bitset_;// = std::bitset<EventDataTypeList::size>{}.set();
//};