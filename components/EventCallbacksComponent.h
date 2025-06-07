#pragma once
#include "BaseComponent.h"
#include "../events/EventCallback.h"

struct EventCallbacks : BaseComponent<EventCallbacks, 11>
{
	std::unordered_map<uint32_t, std::vector<EventCallback>> map;
};
