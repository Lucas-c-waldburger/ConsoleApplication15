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

	//template <SomeEventData T>
	//auto Add(std::string_view name, std::optional<Entity_t> owner = {})
	//{
	//	return this->emplace(T::eventType, 
	//		EventCallbackRegistry::Key{
	//			.eventType = T::eventType, 
	//			.callbackName = std::string{ name }, 
	//			.uniqueOwner = owner
	//		}
	//	);
	//}

	//auto Add(const EventCallbackRegistry::Key& key)
	//{
	//	return this->emplace(key.eventType, key);
	//}

	//auto Add(EventCallbackRegistry::Key&& key)
	//{
	//	return this->emplace(key.eventType, std::move(key));
	//}
};


struct EventCallbacks : BaseComponent<EventCallbacks, 11>
{
	EventCallbackKeyTable table;
};


