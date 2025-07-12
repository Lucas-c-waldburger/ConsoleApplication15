#pragma once
#include <cassert>
#include "EventCallback.h"
#include "../ecs/Ecs.h"
#include "../events/EventUtils.h"
#include "../scripting/TypedLuaFunction.h"

class EventCallbackRegistry
{
public:
	EventCallbackRegistry() = default;
	~EventCallbackRegistry() = default;

	EventCallbackRegistry(const EventCallbackRegistry&) = delete;
	EventCallbackRegistry& operator=(const EventCallbackRegistry&) = delete;

	EventCallbackRegistry(EventCallbackRegistry&& rhs) noexcept : 
		masterTable_(std::move(rhs.masterTable_)) {}
	EventCallbackRegistry& operator=(EventCallbackRegistry&& other) noexcept
	{
		if (this != &other)
		{
			masterTable_ = std::move(other.masterTable_);
		}
		return *this;
	}

	template <typename Fn> requires EventCallbackFnCompatible<Fn>
	std::pair<uint32_t, EventCallbackView> RegisterCallback(std::string_view callbackName, Fn&& callbackFn)
	{
		using EventDataT = std::remove_cvref_t<type_at_index_t<1, typename func_traits<Fn>::arg_types>>;

		HashName callbackNameHash{ callbackName };

		if (masterTable_[EventDataT::eventType].contains(callbackNameHash))
		{
			return {};
		}

		auto wrapped = [fn = std::forward<Fn>(callbackFn)](Entity& entity, const Event& ev) -> ReturnSignal {
			if (const auto* castEv = EventDataCast<EventDataT>(ev))
			{
				return std::invoke(fn, entity, *castEv);
			}

			return ReturnSignal::KeepObserving;
		};

		auto [it, inserted] = masterTable_[EventDataT::eventType].emplace(callbackNameHash, std::move(wrapped));
		assert(inserted);

		return std::make_pair(EventDataT::eventType, EventCallbackView{ callbackNameHash, it->second });
	}

	template <SomeEventData T>
	std::pair<uint32_t, EventCallbackView> RegisterCallback(std::string_view callbackName,
															TypedLuaFunction<ReturnSignal(Entity&, const T&)> luaFn)
	{
		HashName callbackNameHash{ callbackName };

		if (masterTable_[T::eventType].contains(callbackNameHash))
		{
			return {};
		}

		auto wrapped = [fn = std::move(luaFn)](Entity& entity, const Event& ev) -> ReturnSignal {
			if (const auto* castEv = EventDataCast<T>(ev))
			{
				auto result = fn(entity, *castEv);
				if (!result.Success())
				{
					LOG_ERROR(result.GetError());

					return ReturnSignal::StopObserving;
				}

				return result.GetValue();
			}

			return ReturnSignal::KeepObserving;
		};

		auto [it, inserted] = masterTable_[T::eventType].emplace(callbackNameHash, std::move(wrapped));
		assert(inserted);

		return std::make_pair(T::eventType, EventCallbackView{ callbackNameHash, it->second });
	}

	bool EraseCallback(uint32_t eventType, std::string_view callbackName)
	{
		HashName callbackNameHash{ callbackName };

		auto it = masterTable_[eventType].find(callbackNameHash);
		if (it == masterTable_[eventType].end())
		{
			return false;
		}

		masterTable_[eventType].erase(it);

		return true;
	}

	std::pair<uint32_t, EventCallbackView> GetCallback(uint32_t eventType, HashName callbackNameHash)
	{
		auto it = masterTable_[eventType].find(callbackNameHash);
		if (it == masterTable_[eventType].end())
		{
			return {};
		}

		return std::make_pair(eventType, EventCallbackView{ callbackNameHash, it->second });
	}

	std::pair<uint32_t, EventCallbackView> GetCallback(uint32_t eventType, std::string_view callbackName)
	{
		return GetCallback(eventType, HashName{ callbackName });
	}

private:
	std::array<std::unordered_map<HashName, EventCallback>, EventDataTypeList::size> masterTable_;
};