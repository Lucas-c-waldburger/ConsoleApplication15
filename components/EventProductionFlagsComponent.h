#pragma once
#include "BaseComponent.h"
#include "../core/EventProductionFlags.h"

// If entity does not have EventProductionFlags, will produce all events
// all event types are opt-out, must specify which events to disable producing
// obviously, an entity wont produce all event data types, but it makes it flexible

struct EntityEventProductionFlags : public BaseComponent<EntityEventProductionFlags>
{
	//std::bitset<EventDataTypeList::size> flags = std::bitset<EventDataTypeList::size>{}.set();
	EventProductionFlags<EventDataTypeList> flags;
};
