#pragma once
#include "EventDataTypeList.h"

template <typename Derived> requires type_in_list_v<Derived, EventDataTypeList>
struct IEventData
{
	static_assert(index_of_v<Derived, EventDataTypeList> < static_cast<size_t>(std::numeric_limits<uint32_t>::max()));

	// generate our event type int from <Derived>'s index in the EventDataTypeList
	static constexpr uint32_t eventType = index_of_v<Derived, EventDataTypeList>;

	uint32_t timestamp = 0;
};