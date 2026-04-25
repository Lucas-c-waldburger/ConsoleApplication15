#pragma once
#include "../../events/EventBus2.h"

class EventContext
{
public:
	static inline EventBus* eventBus = nullptr;

private:
	EventContext() = default;
};