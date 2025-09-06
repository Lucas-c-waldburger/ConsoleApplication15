#pragma once
#include <cassert>
#include "EventCallbackHandle.h"
#include "../ecs/Ecs.h"
#include "../events/EventUtils.h"
#include "../scripting/TypedLuaFunction.h"
#include "EventCallbackMasterTable.h"

class EventCallbackRegistry
{
public:
	EventCallbackRegistry() = default;
	~EventCallbackRegistry() = default;

	EventCallbackRegistry(const EventCallbackRegistry&) = delete;
	EventCallbackRegistry& operator=(const EventCallbackRegistry&) = delete;

	EventCallbackRegistry(EventCallbackRegistry&& rhs) noexcept :
		masterTable_(std::move(rhs.masterTable_)) {
	}
	EventCallbackRegistry& operator=(EventCallbackRegistry&& other) noexcept
	{
		if (this != &other)
		{
			masterTable_ = std::move(other.masterTable_);
		}
		return *this;
	}

	template <typename Fn> requires EventCallbackFnCompatible<Fn>
	Handle<EventCallback> RegisterCallback(std::string_view callbackName, Fn&& callbackFn)
	{
		return masterTable_.Insert(
			EventCallback{ callbackName, std::forward<Fn>(callbackFn) }
		);
	}

	template <SomeEventData T>
	Handle<EventCallback> RegisterCallback(std::string_view callbackName,
										   TypedLuaFunction<ReturnSignal(Entity&, const T&)> luaFn)
	{
		return masterTable_.Insert(
			EventCallback{ callbackName, luaFn }
		);
	}

	template <typename Fn> requires EventCallbackFnCompatible<Fn>
	Handle<EventCallback> RegisterOrRetrieveCallback(std::string_view callbackName, Fn&& callbackFn)
	{
		const uint32_t eventType = ExtractEventDataTypeFromFnArgs<Fn>::eventType;
		
		auto handle = masterTable_.GetCallbackHandle(eventType, callbackName);

		if (!handle.IsValid())
		{
			handle = masterTable_.Insert(
				EventCallback{ callbackName, std::forward<Fn>(callbackFn) }
			);
		}

		return handle;
	}

	bool EraseCallback(uint32_t eventType, std::string_view callbackName)
	{
		return masterTable_.Erase(eventType, callbackName);
	}

	Result<EventCallback::View> GetCallbackView(uint32_t eventType, std::string_view callbackName) const
	{
		return masterTable_.GetCallbackView(eventType, callbackName);
	}
	Result<EventCallback::View> GetCallbackView(const Handle<EventCallback>& handle) const
	{
		return masterTable_.GetCallbackView(handle);
	}

	bool HasCallback(uint32_t eventType, std::string_view callbackName) const
	{
		return masterTable_.Contains(eventType, callbackName);
	}
	bool HasCallback(const Handle<EventCallback>& handle) const
	{
		return HasCallback(handle);
	}

private:
	EventCallbackMasterTable masterTable_;
};