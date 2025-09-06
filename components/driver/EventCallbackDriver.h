#pragma once
#include "BaseDriver.h"
#include "../../core/Algorithms.h"

//class EventCallbackDriver : public BaseDriver<EventCallbackDriver, EventCallbacks>
//{
//public:
//	friend class Super;
//
//	void AddCallback(uint32_t eventType, std::string_view callbackName)
//	{
//		auto& callbacks = GetComponent<EventCallbacks>();
//
//		callbacks.table[eventType].emplace_back(HashName{ callbackName });
//	}
//
//	template <SomeEventData T>
//	void AddCallback(std::string_view callbackName)
//	{
//		return AddCallback(T::eventType, callbackName);
//	}
//
//	bool ClearCallback(uint32_t eventType, std::string_view callbackName)
//	{
//		auto& callbacks = GetComponent<EventCallbacks>();
//
//		auto it = callbacks.table.find(eventType);
//		if (it == callbacks.table.end())
//		{
//			return false;
//		}
//
//		return Erase(it->second, HashName{ callbackName });
//	}
//
//	template <SomeEventData T>
//	bool ClearCallback(std::string_view callbackName)
//	{
//		return ClearCallback(T::eventType, callbackName);
//	}
//
//	bool ClearCallbacks(uint32_t eventType)
//	{
//		auto& callbacks = GetComponent<EventCallbacks>();
//
//		return callbacks.table.erase(eventType);
//	}
//
//	template <SomeEventData T>
//	bool ClearCallbacks()
//	{
//		return ClearCallbacks(T::eventType);
//	}
//
//private:
//	explicit EventCallbackDriver(Entity ent) : BaseDriver(ent) {}
//	//EventCallbackDriver(Entity ent, EventCallbackRegistry& registry) : 
//	//	BaseDriver(ent), registry_(&registry) {}
//
//	//EventCallbackRegistry* registry_ = nullptr;
//};