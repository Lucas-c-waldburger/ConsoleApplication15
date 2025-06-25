#pragma once
#include "BaseComponent.h"
#include "../events/EventCallbackRegistry.h"


struct EventCallbackKeyTable : std::unordered_map<uint32_t, EventCallbackRegistry::Key>
{
	auto AddKey(const EventCallbackRegistry::Key& key)
	{
		return this->emplace(key.eventType, key);
	}
	auto AddKey(EventCallbackRegistry::Key&& key)
	{
		return this->emplace(key.eventType, std::move(key));
	}
};


//struct EventCallbacks : BaseComponent<EventCallbacks, 11>
struct EventCallbacks : BaseComponent<EventCallbacks>
{
	EventCallbackKeyTable table;
};


