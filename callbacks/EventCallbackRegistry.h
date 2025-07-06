#pragma once
#include "CallbackRegistryTable.h"
#include "../events/EventBus.h"
#include "../events/EventUtils.h"
#include "../ecs/EntityT.h"
#include "../scripting/TypedLuaFunction.h"
#include "../core/TransparentStringHash.h"
#include "../core/FuncTraits.h"
#include "../core/CommonFunctions.h"
#include "../inputs/controller/GameControllerInputSource.h"

struct EventCallbackDescriptor : BaseCallbackDescriptor
{
	EventCallbackDescriptor() = default;
	EventCallbackDescriptor(uint32_t evType, std::string_view name, std::optional<Entity_t> owner) :
		BaseCallbackDescriptor{ std::string{name}, owner }, eventType(evType) {}

	uint32_t eventType = kInvalidEventType;
	bool operator==(const EventCallbackDescriptor& rhs) const
	{
		return eventType == rhs.eventType && 
			   static_cast<const BaseCallbackDescriptor&>(*this) == static_cast<const BaseCallbackDescriptor&>(rhs);
	}
};

namespace std {
	template <>
	struct hash<EventCallbackDescriptor> {
		size_t operator()(const EventCallbackDescriptor& desc) const noexcept {
			size_t hash = 0;
			HashCombine(hash, std::hash<uint32_t>{}(desc.eventType));
			HashCombine(hash, std::hash<BaseCallbackDescriptor>{}(static_cast<const BaseCallbackDescriptor&>(desc)));

			return hash;
		}
	};
}

using EventCallbackRegistryTable = CallbackRegistryTable<EventCallbackDescriptor, const Event&>;
using EventCallbackFn = EventCallbackRegistryTable::CallbackFn;
using EventCallbackFnView = EventCallbackRegistryTable::CallbackFnView;

namespace detail {
template <typename Fn>
struct is_event_callback_fn_compatible
{
	static_assert(HasFuncTraits<Fn>);
	static_assert(func_traits<Fn>::arg_types::size == 2);

	using Ret = typename func_traits<Fn>::return_type;
	using FirstArg = type_at_index_t<0, typename func_traits<Fn>::arg_types>;
	using SecondArg = type_at_index_t<1, typename func_traits<Fn>::arg_types>;

	static constexpr bool value =
		std::same_as<Ret, ReturnSignal> &&
		std::same_as<FirstArg, Entity_t> &&
		SomeEventData<std::remove_cvref_t<SecondArg>>&& is_const_reference_v<SecondArg>;
};
} // detail

template <typename Fn>
concept EventCallbackFnCompatible = detail::is_event_callback_fn_compatible<Fn>::value;

class EventCallbackRegistry
{
public:
	struct RegistrationOutcome
	{
		uint32_t eventType = kInvalidEventType;
		Handle<EventCallbackDescriptor> handle = {};
		bool newlyRegistered = false;
	};

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

	// Register with name, callable, owner
	template <typename Fn> requires EventCallbackFnCompatible<Fn>
	RegistrationOutcome RegisterCallback(std::string_view name, Fn&& fn, std::optional<Entity_t> owner)
	{
		using EventDataT = std::remove_cvref_t<type_at_index_t<1, typename func_traits<Fn>::arg_types>>;

		EventCallbackDescriptor desc{ EventDataT::eventType, name, owner };

		if (masterTable_.Contains(desc))
		{
			LOG_INFO_FMT("Callback named '{}' already registered "
				"for event type and owner", desc.callbackName);

			return { 
				.eventType = EventDataT::eventType,
				.handle = masterTable_.GetHandle(desc), 
				.newlyRegistered = false 
			};
		}

		auto rawCallback = [fn = std::forward<Fn>(fn)](Entity_t ent, const Event& ev) -> ReturnSignal {
			if (const auto* castEv = EventDataCast<EventDataT>(ev))
			{
				return std::invoke(fn, ent, *castEv);
			}

			return ReturnSignal::KeepObserving;
		};

		auto handle = masterTable_.Insert(std::move(desc), std::move(rawCallback));
		assert(handle.IsValid());

		return { 
			.eventType = EventDataT::eventType,
			.handle = handle, 
			.newlyRegistered = true 
		};
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
										 const TypedLuaFunction<ReturnSignal(Entity_t, const T&)>& luaFn,
										 std::optional<Entity_t> owner)
	{
		EventCallbackDescriptor desc{ T::eventType, name, owner };

		if (masterTable_.Contains(desc))
		{
			LOG_INFO_FMT("Callback named '{}' already registered "
				"for event type and owner", desc.callbackName);

			return { 
				.eventType = T::eventType,
				.handle = masterTable_.GetHandle(desc), 
				.newlyRegistered = false 
			};
		}

		auto rawCallback = [fn = luaFn](Entity_t ent, const Event& ev) -> ReturnSignal {
			if (const auto* castEv = EventDataCast<T>(ev))
			{
				auto result = fn(ent, *castEv);
				if (!result.Success())
				{
					LOG_ERROR(result.GetError());

					return ReturnSignal::StopObserving;
				}

				return result.GetValue();
			}

			return ReturnSignal::KeepObserving;
		};

		auto handle = masterTable_.Insert(std::move(desc), std::move(rawCallback));
		assert(handle.IsValid());

		return { 
			.eventType = T::eventType,
			.handle = handle, 
			.newlyRegistered = true 
		};
	}

	// Register with name, lua function
	template <SomeEventData T>
	RegistrationOutcome RegisterCallback(std::string_view name,
										 const TypedLuaFunction<ReturnSignal(Entity_t, const T&)>& luaFn)
	{
		return RegisterCallback(name, luaFn, {});
	}

	EventCallbackFnView GetCallbackView(const Handle<EventCallbackDescriptor>& handle)
	{
		return masterTable_.GetCallbackView(handle);
	}

	const EventCallbackDescriptor* GetCallbackDescriptor(const Handle<EventCallbackDescriptor>& handle) const
	{
		return masterTable_.GetCallbackDescriptor(handle);
	}

	bool RemoveCallback(const Handle<EventCallbackDescriptor>& handle)
	{
		return masterTable_.Erase(handle);
	}

	size_t RemoveCallbacksWithOwner(Entity_t owner)
	{
		return masterTable_.EraseAllWithOwner(owner);
	}

	//std::vector<EventCallbackDescriptor> FindCallbackDetailsByName(std::string_view name) const
	//{
	//	return masterTable_.FindCallbackDetailsByName(name);
	//}

	//std::vector<EventCallbackDescriptor> FindCallbackDetailsByOwner(Entity_t owner) const
	//{
	//	return masterTable_.FindCallbackDetailsByOwner(owner);
	//}

private:
	EventCallbackRegistryTable masterTable_;
};