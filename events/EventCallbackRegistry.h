#pragma once
#include <vector>
#include "../events/EventBus.h"
#include "../events/EventUtils.h"
#include "../ecs/EntityT.h"
#include "EventCallbackFn.h"
#include "../scripting/TypedLuaFunction.h"
#include "../core/TransparentStringHash.h"
#include "../core/CommonFunctions.h"

class EventCallbackRegistry
{
private:
	//struct InternalKey
	//{
	//	std::string callbackName;
	//	std::optional<Entity_t> uniqueOwner; // says "remove this callback from registry if owner is destroyed"
	//										 // also gives key uniqueness to same fn names with different owners
	//	friend bool operator==(const InternalKey& lhs, const InternalKey& rhs) 
	//	{
	//		return lhs.callbackName == rhs.callbackName && lhs.uniqueOwner == rhs.uniqueOwner;
	//	}
	//};

public:
	//struct Key : InternalKey
	//{
	//	Key() = default;
	//	Key(std::string_view nm, std::optional<Entity_t> owner = {}) :
	//		InternalKey{ std::string{nm}, owner } {}
	//	Key(uint32_t evType, std::string_view nm, std::optional<Entity_t> owner = {}) :
	//		InternalKey{ std::string{nm}, owner }, eventType(evType) {}

	//	uint32_t eventType;
	//	operator const InternalKey& () 
	//	{ 
	//		return *this; 
	//	}
	//};
	struct Key
	{
		uint32_t eventType = kInvalidEventType;
		std::string callbackName;
		std::optional<Entity_t> uniqueOwner; // says "remove this callback from registry if owner is destroyed"
											 // also gives key uniqueness to same fn names with different owners
		friend bool operator==(const Key& lhs, const Key& rhs)
		{
			return lhs.eventType == rhs.eventType &&
				   lhs.callbackName == rhs.callbackName &&
				   lhs.uniqueOwner == rhs.uniqueOwner;
		}
	};

	//using RegistrationOutcome = std::pair<Key, bool>;
	//static constexpr std::string_view kAlreadyRegistered = "already registered";
	struct RegistrationOutcome
	{
		Key key;
		bool newlyRegistered = false;
	};

	EventCallbackRegistry() = default;
	~EventCallbackRegistry() = default;

	EventCallbackRegistry(const EventCallbackRegistry&) = delete;
	EventCallbackRegistry& operator=(const EventCallbackRegistry&) = delete;

	EventCallbackRegistry(EventCallbackRegistry&& rhs) noexcept : masterTable_(std::move(rhs.masterTable_)) {}
	EventCallbackRegistry& operator=(EventCallbackRegistry&& rhs) noexcept
	{
		if (this != &rhs)
		{
			this->masterTable_ = std::move(rhs.masterTable_);
		}
		return *this;
	}

	// Register with name, callable, owner
	template <typename Fn> requires EventCallbackFnCompatible<Fn>
	RegistrationOutcome RegisterCallback(std::string_view name, Fn&& fn, std::optional<Entity_t> owner)
	{
		using EventDataT = std::remove_cvref_t<type_at_index_t<0, typename func_traits<Fn>::arg_types>>;

		RegistrationOutcome outcome{
			.key = { EventDataT::eventType, std::string{name}, owner },
			.newlyRegistered = false
		};

		if (KeyRegistered(outcome.key))
		{
			LOG_ERROR_FMT("Callback named '{}' already registered "
				"for event type and owner", outcome.key.callbackName);

			return outcome;
		}

		auto rawCallback = [fn = std::forward<Fn>(fn)](const Event& ev, Entity_t ent) -> ReturnSignal {
			if (const auto* castEv = EventDataCast<EventDataT>(ev))
			{
				return std::invoke(fn, *castEv, ent);
			}

			return ReturnSignal::KeepObserving;
		};

		auto [it, inserted] = masterTable_[EventDataT::eventType].emplace(outcome.key, std::move(rawCallback));
		assert(inserted);

		outcome.newlyRegistered = true;

		return outcome;
	}

	// Register with name, callable
	template <typename Fn> requires EventCallbackFnCompatible<Fn>
	RegistrationOutcome RegisterCallback(std::string_view name, Fn&& fn)
	{
		return RegisterCallback(name, std::forward<Fn>(fn), {});
	}

	// Register with name, lua function, owner
	template <SomeEventData T>
	RegistrationOutcome RegisterCallback(std::string_view name,
										 const TypedLuaFunction<ReturnSignal(const T&, Entity_t)>& luaFn,
										 std::optional<Entity_t> owner)
	{
		RegistrationOutcome outcome{
			.key = { T::eventType, std::string{name}, owner },
			.newlyRegistered = false
		};

		if (KeyRegistered(outcome.key))
		{
			LOG_ERROR_FMT("Callback named '{}' already registered "
				"for event type and owner", outcome.key.callbackName);

			return outcome;
		}

		auto rawCallback = [fn = luaFn](const Event& ev, Entity_t ent) -> ReturnSignal {
			if (const auto* castEv = EventDataCast<T>(ev))
			{
				auto result = fn(*castEv, ent);
				if (!result.Success())
				{
					LOG_ERROR(result.GetError());

					return ReturnSignal::StopObserving;
				}

				return result.GetValue();
			}

			return ReturnSignal::KeepObserving;
		};

		auto [it, inserted] = masterTable_[T::eventType].emplace(outcome.key, std::move(rawCallback));
		assert(inserted);

		outcome.newlyRegistered = true;

		return outcome;
	}

	// Register with name, lua function
	template <SomeEventData T>
	RegistrationOutcome RegisterCallback(std::string_view name,
										 const TypedLuaFunction<ReturnSignal(const T&, Entity_t)>& luaFn)
	{
		return RegisterCallback(name, luaFn, {});
	}

	// Register with pre-made key, callable
	template <typename Fn> requires EventCallbackFnCompatible<Fn>
	RegistrationOutcome RegisterCallback(const Key& key, Fn&& fn)
	{
		return RegisterCallback(key.callbackName, std::forward<Fn>(fn), key.uniqueOwner);
	}

	// Register with pre-made key, lua function
	template <SomeEventData T>
	RegistrationOutcome RegisterCallback(const Key& key,
										 const TypedLuaFunction<ReturnSignal(const T&, Entity_t)>& luaFn)
	{
		return RegisterCallback(key.callbackName, luaFn, key.uniqueOwner);
	}

	EventCallbackView GetCallback(const Key& key) const
	{
		if (KeyRegistered(key))
		{
			return EventCallbackView{ masterTable_[key.eventType].at(key) };
		}

		return EventCallbackView{};
	}

	bool RemoveCallback(const Key& key)
	{
		if (KeyRegistered(key))
		{
			return masterTable_[key.eventType].erase(key); 
		}

		return false;
	}

	bool KeyRegistered(const Key& key) const
	{
		if (key.eventType < masterTable_.size() && !key.callbackName.empty())
		{
			auto it = masterTable_[key.eventType].find(key);

			return it != masterTable_[key.eventType].end() && it->second != nullptr;
		}

		return false;
	}

	std::vector<Key> FindKeysWithName(std::string_view name) const
	{
		return FindKeysImpl(std::string{name}, &Key::callbackName);
	}

	std::vector<Key> FindKeysWithOwner(Entity_t owner) const
	{
		return FindKeysImpl(std::optional<Entity_t>{owner}, &Key::uniqueOwner);
	}

private:
	struct KeyHash
	{
		size_t operator()(const Key& k) const noexcept
		{
			size_t hash = 0;
			HashCombine(hash, std::hash<std::string>{}(k.callbackName));
			HashCombine(hash, std::hash<std::optional<Entity_t>>{}(k.uniqueOwner));

			return hash;
		}
	};
	struct KeyEq
	{
		bool operator()(const Key& lhs, const Key& rhs) const
		{
			return lhs.callbackName == rhs.callbackName &&
				   lhs.uniqueOwner  == rhs.uniqueOwner;
		}
	};
	using KeyMap = std::unordered_map<Key, EventCallbackFn, KeyHash, KeyEq>;

	template <typename T>
	std::vector<Key> FindKeysImpl(const T& comp, T Key::*member) const
	{
		std::vector<Key> keys;
		for (size_t i = 0; i < masterTable_.size(); i++)
		{
			for (const auto& entry : masterTable_[i])
			{
				if (comp == entry.first.*member)
				{
					keys.emplace_back(entry.first);
				}
			}
		}

		return keys;
	}

	std::array<KeyMap, EventDataTypeList::size> masterTable_;
};

