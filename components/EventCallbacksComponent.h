#pragma once
#include "BaseComponent.h"
#include "../callbacks/EventCallback.h"

struct EventCallbacks : BaseComponent<EventCallbacks>
{
	std::unordered_map<uint32_t, EventCallbackView> table;
};

