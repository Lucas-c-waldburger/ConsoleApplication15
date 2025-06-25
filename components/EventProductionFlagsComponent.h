#pragma once
#include <bitset>
#include "BaseComponent.h"
#include "../events/EventUtils.h"

// If entity does not have EventProductionFlags, will produce all events
// all event types are opt-out, must specify which events to disable producing
// obviously, an entity wont produce all event data types, but it makes it flexible
//struct EventProductionFlags : public BaseComponent<EventProductionFlags, 15>
struct EventProductionFlags : public BaseComponent<EventProductionFlags>
{
	std::bitset<EventDataTypeList::size> flags = std::bitset<EventDataTypeList::size>{}.set();
};
