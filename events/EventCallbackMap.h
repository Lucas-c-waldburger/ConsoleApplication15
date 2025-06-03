#pragma once
#include <unordered_map>
#include <vector>
#include "EventCallback.h"
#include "../core/Algorithms.h"


class EventCallbackMap
{
public:
	template <SomeCustomEvent T, typename Fn>
	Handle<EventCallback> Insert(Fn&& fn)
	{
		auto handle = Handle<EventCallback>::Create<T>();

		map_[T::GetEventType()].emplace_back(std::forward<Fn>(fn), handle);

		return handle;
	}

	bool Erase(const Handle<EventCallback>& handle);

	bool HasCallbacks(uint32_t eventType) const
	{
		auto it = map_.find(eventType);
		
		return (it != map_.end()) ? !it->second.empty() : false;
	}

	template <SomeCustomEvent T>
	bool HasCallbacks() const
	{
		return HasCallbacks(T::GetEventType());
	}

	std::vector<EventCallback>& GetCallbacks(uint32_t eventType)
	{
		return map_[eventType];
	}

	template <SomeCustomEvent T>
	std::vector<EventCallback>& GetCallbacks()
	{
		return GetCallbacks(T::GetEventType());
	}

private:
	std::unordered_map<uint32_t, std::vector<EventCallback>> map_;
};
