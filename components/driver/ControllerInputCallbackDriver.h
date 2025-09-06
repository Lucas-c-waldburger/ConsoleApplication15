#pragma once
#include "BaseDriver.h"
#include "../GameControllerInputCallbacksComponent.h"
#include "../../core/Algorithms.h"

//class ControllerInputCallbackDriver : public BaseDriver<ControllerInputCallbackDriver, 
//														GameControllerInputCallbacks>
//{
//public:
//	friend class Super;
//
//	using Source = GameControllerInputSource;
//
//	void AddCallback(Source src, std::string_view callbackName)
//	{
//		auto& callbacks = GetComponent<GameControllerInputCallbacks>();
//
//		callbacks.table[src].emplace_back(HashName{ callbackName });
//	}
//
//	template <Source src>
//	void AddCallback(std::string_view callbackName)
//	{
//		return AddCallback(src, callbackName);
//	}
//
//	bool ClearCallback(Source src, std::string_view callbackName)
//	{
//		auto& callbacks = GetComponent<GameControllerInputCallbacks>();
//
//		auto it = callbacks.table.find(src);
//		if (it == callbacks.table.end())
//		{
//			return false;
//		}
//
//		return Erase(it->second, HashName{ callbackName });
//	}
//
//	template <Source src>
//	bool ClearCallback(std::string_view callbackName)
//	{
//		return ClearCallback(src, callbackName);
//	}
//
//	bool ClearCallbacks(Source src)
//	{
//		auto& callbacks = GetComponent<GameControllerInputCallbacks>();
//
//		return callbacks.table.erase(src);
//	}
//
//	template <Source src>
//	bool ClearCallbacks()
//	{
//		return ClearCallbacks(src);
//	}
//
//private:
//	explicit ControllerInputCallbackDriver(Entity ent) : BaseDriver(ent) {}
//};
