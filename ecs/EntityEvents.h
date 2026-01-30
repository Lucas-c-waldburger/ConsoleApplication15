#pragma once
#include "Ecs.h"
#include "../events/EventBus2.h"
//#include "../user/UserEventBridge.h"
#include "../core/FuncTraits.h"
#include "../events/data/EntityEventConcept.h"

namespace detail {

	template <typename Tup>
	struct no_args_are_entities;

	template <template <typename...> class Tup, typename...Ts>
	struct no_args_are_entities<Tup<Ts...>> :
		std::bool_constant<((!std::same_as<std::remove_cvref_t<Ts>, Entity>) && ...)> {
	};

	template <typename Tup>
	struct all_args_refs;

	template <template <typename...> class Tup, typename...Ts>
	struct all_args_refs<Tup<Ts...>> :
		std::bool_constant<(is_non_const_reference_v<Ts> && ...)> {
	};

	template <typename Tup>
	struct all_args_const_refs;

	template <template <typename...> class Tup, typename...Ts>
	struct all_args_const_refs<Tup<Ts...>> :
		std::bool_constant<(is_const_reference_v<Ts> && ...)> {
	};

	template <typename Tup> struct get_components_helper;

	template <template <typename...> class Tup, typename...ComponentTs>
	struct get_components_helper<Tup<ComponentTs...>>
	{
		template <typename Ev, typename Fn>
		static void call(Fn&& fn, const Ev& ev, Entity& e)
		{
			if (!e.HasComponents<std::remove_cvref_t<ComponentTs>...>())
			{
				return;
			}

			std::invoke(fn, ev, e.GetComponent<std::remove_cvref_t<ComponentTs>>()...);
		}
		template <typename Ev, typename Fn>
		static void call(Fn&& fn, const Ev& ev, const Entity& e)
		{
			if (!e.HasComponents<std::remove_cvref_t<ComponentTs>...>())
			{
				return;
			}

			std::invoke(fn, ev, e.GetComponent<std::remove_cvref_t<ComponentTs>>()...);
		}
	};

} // detail

template <typename Tup>
static constexpr bool no_args_are_entities_v = detail::no_args_are_entities<Tup>::value;

template <typename Tup>
static constexpr bool all_args_refs_v = detail::all_args_refs<Tup>::value;

template <typename Tup>
static constexpr bool all_args_const_refs_v = detail::all_args_const_refs<Tup>::value;

template <typename Tup, typename Fn, typename Ev>
inline void ForwardEventCallbackComponents(Fn&& fn, const Ev& ev, Entity& e)
{
	return detail::get_components_helper<Tup>::call(fn, ev, e);
}
template <typename Tup, typename Fn, typename Ev>
inline void ForwardEventCallbackComponents(Fn&& fn, const Ev& ev, const Entity& e)
{
	return detail::get_components_helper<Tup>::call(fn, ev, e);
}

template <typename Fn>
inline constexpr bool fn_returns_void_v =
	std::same_as<typename func_traits<Fn>::return_type, void>;

template <typename Fn>
inline constexpr bool arg_0_is_event_data_v =
	(func_traits<Fn>::argCount > 0 &&
	 SomeEventData<std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>> &&
	 is_const_reference_v<typename func_traits<Fn>::template arg_at<0>>);

template <typename Fn>
inline constexpr bool valid_event_callback_base_sig_v = 
	(fn_returns_void_v<Fn> && func_traits<Fn>::argCount > 0
	 && is_const_reference_v<typename func_traits<Fn>::template arg_at<0>>);

template <typename Fn> requires fn_returns_void_v<Fn>
struct ev_callback_sig
{
	struct Fake {};
	using always_fail_sig = Fake(*)(Fake, Fake);
	using resolved_fn = std::conditional_t<
		func_traits<Fn>::argCount >= 2,
		Fn,
		always_fail_sig
	>;

	static constexpr bool with_event_data_only_v = func_traits<Fn>::argCount == 1;

	static constexpr bool with_entity_v = (func_traits<resolved_fn>::argCount == 2 &&
		std::same_as<typename func_traits<resolved_fn>::template arg_at<1>, Entity&>);

	static constexpr bool with_const_entity_v = (func_traits<Fn>::argCount == 2 &&
		std::same_as<typename func_traits<resolved_fn>::template arg_at<1>, const Entity&>);

	static constexpr bool with_components_v = (func_traits<Fn>::argCount >= 2 &&
		no_args_are_entities_v<pop_front_t<typename func_traits<resolved_fn>::arg_types>> &&
		all_args_refs_v<pop_front_t<typename func_traits<resolved_fn>::arg_types>>);

	static constexpr bool with_const_components_v = (func_traits<Fn>::argCount >= 2 &&
		no_args_are_entities_v<pop_front_t<typename func_traits<resolved_fn>::arg_types>> &&
		all_args_const_refs_v<pop_front_t<typename func_traits<resolved_fn>::arg_types>>);
};

template <typename Fn>
inline constexpr bool valid_event_callback_sig_v = 
(
	 valid_event_callback_base_sig_v<Fn> &&
	 ev_callback_sig<Fn>::with_event_data_only_v ||
	 ev_callback_sig<Fn>::with_entity_v		  ||
	 ev_callback_sig<Fn>::with_const_entity_v    ||
	 ev_callback_sig<Fn>::with_components_v	  ||
	 ev_callback_sig<Fn>::with_const_components_v
);

//template <typename Fn>
//inline constexpr bool valid_user_event_callback_sig_v =
//(
//	!arg_0_is_event_data_v<Fn> &&
//	ev_callback_sig<Fn>::with_event_data_only_v ||
//	ev_callback_sig<Fn>::with_entity_v ||
//	ev_callback_sig<Fn>::with_const_entity_v ||
//	ev_callback_sig<Fn>::with_components_v ||
//	ev_callback_sig<Fn>::with_const_components_v
//);
	 
template <typename Src, typename Fn>
inline constexpr bool valid_input_callback_sig_v = (
	valid_event_callback_sig_v<Fn> && 
	SomeInputEvent<std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>> &&
	std::same_as<extract_input_event_src_type_t<
		std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>>, Src>
);

template <SomeEntityEvent Ev>
inline bool IsEntityInvolvedInEvent(const Entity& entity, const Ev& event)
{
	static constexpr auto entityMatchesOne =
	[]<size_t I>(const Entity& e, const Ev& ev) {
		return e.GetID() == ev.entity<I>();
	};

	static constexpr auto entityMatchesAny =
	[]<size_t...Is>(const Entity& e, const Ev& ev, std::index_sequence<Is...>) {
		return (entityMatchesOne.template operator()<Is>(e, ev) || ...);
	};

	return entityMatchesAny(entity, event, std::make_index_sequence<Ev::entityCount>{});
}

template <typename Fn> requires valid_event_callback_sig_v<Fn>
static Result<Void> OnEventImpl(EventBus2& bus, Entity& e, Fn&& fn)
{
	if (!e.IsValid())
	{
		return MAKE_ERROR("Entity was invalid");
	}

	using event_data_t = 
		std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>;

	auto& tks = e.AddComponent<SignalTokenStorage>().signalTokens;

	if constexpr (ev_callback_sig<Fn>::with_event_data_only_v)
	{
		tks.emplace_back(bus.ConnectToEvent(
			[e, f = std::forward<Fn>(fn)](const event_data_t& ev) {
				if (!e.IsValid())
				{
					return;
				}
				if constexpr (SomeEntityEvent<event_data_t>)
				{
					if (!IsEntityInvolvedInEvent(e, ev))
					{
						return;
					}
				}

				std::invoke(f, ev);
			}));

		return Void{};
	}
	else if constexpr (ev_callback_sig<Fn>::with_entity_v || 
					   ev_callback_sig<Fn>::with_const_entity_v)
	{
		tks.emplace_back(bus.ConnectToEvent(
			[e, f = std::forward<Fn>(fn)](const event_data_t& ev) mutable {
				if (!e.IsValid())
				{
					return;
				}
				if constexpr (SomeEntityEvent<event_data_t>)
				{
					if (!IsEntityInvolvedInEvent(e, ev))
					{
						return;
					}
				}

				std::invoke(f, ev, e);
			}));

		return Void{};
	}
	else if constexpr (ev_callback_sig<Fn>::with_components_v || 
					   ev_callback_sig<Fn>::with_const_components_v)
	{
		using cmps = pop_front_t<typename func_traits<Fn>::arg_types>;

		tks.emplace_back(bus.ConnectToEvent(
			[e, f = std::forward<Fn>(fn)](const event_data_t& ev) mutable {
				if (!e.IsValid())
				{
					return;
				}
				if constexpr (SomeEntityEvent<event_data_t>)
				{
					if (!IsEntityInvolvedInEvent(e, ev))
					{
						return;
					}
				}

				ForwardEventCallbackComponents<cmps>(std::forward<Fn>(f), ev, e);
			}));

		return Void{};
	}
	else
	{
		return MAKE_ERROR("Internal inconsistency - event callback signature "
			"did not conform to any valid signatures");
	}
}

template <typename Src, typename Fn> requires valid_input_callback_sig_v<Src, Fn>
static Result<Void> OnInputImpl(EventBus2& bus, Src src, Entity& e, Fn&& fn)
{
	using event_data_t =
		std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>;

	if (!e.IsValid())
	{
		return MAKE_ERROR("Entity was invalid");
	}

	auto& tks = e.AddComponent<SignalTokenStorage>().signalTokens;

	if constexpr (ev_callback_sig<Fn>::with_event_data_only_v)
	{
		tks.emplace_back(bus.ConnectToInput(src,
			[e, f = std::forward<Fn>(fn)](const event_data_t& ev) {
				if (!e.IsValid())
				{
					return;
				}
				if constexpr (SomeEntityEvent<event_data_t>)
				{
					if (!IsEntityInvolvedInEvent(e, ev))
					{
						return;
					}
				}

				std::invoke(f, ev);
			}));

		return Void{};
	}
	else if constexpr (ev_callback_sig<Fn>::with_entity_v ||
					   ev_callback_sig<Fn>::with_const_entity_v)
	{
		tks.emplace_back(bus.ConnectToInput(src,
			[e, f = std::forward<Fn>(fn)](const event_data_t& ev) mutable {
				if (!e.IsValid())
				{
					return;
				}
				if constexpr (SomeEntityEvent<event_data_t>)
				{
					if (!IsEntityInvolvedInEvent(e, ev))
					{
						return;
					}
				}

				std::invoke(f, ev, e);
			}));

		return Void{};
	}
	else if constexpr (ev_callback_sig<Fn>::with_components_v ||
					   ev_callback_sig<Fn>::with_const_components_v)
	{
		using cmps = pop_front_t<typename func_traits<Fn>::arg_types>;

		tks.emplace_back(bus.ConnectToInput(src,
			[e, f = std::forward<Fn>(fn)](const event_data_t& ev) mutable {
				if (!e.IsValid())
				{
					return;
				}
				if constexpr (SomeEntityEvent<event_data_t>)
				{
					if (!IsEntityInvolvedInEvent(e, ev))
					{
						return;
					}
				}

				ForwardEventCallbackComponents<cmps>(std::forward<Fn>(f), ev, e);
			}));

		return Void{};
	}
	else
	{
		return MAKE_ERROR("Internal inconsistency - input callback signature "
			"did not conform to any valid signatures");
	}
}

class EntityEvents
{
public:
	EntityEvents() = default;
	EntityEvents(const Entity& e, EventBus2* bus) : entity_(e), bus_(bus) {}

	template <typename Fn> requires valid_event_callback_sig_v<Fn>
	Result<Void> OnEvent(Fn&& fn)
	{
		if (!bus_)
		{
			return MAKE_ERROR("Internal EventBus was null");
		}

		return OnEventImpl(*bus_, entity_, std::forward<Fn>(fn));
	}

	template <typename Src, typename Fn> requires valid_input_callback_sig_v<Src, Fn>
	Result<Void> OnInput(Src src, Fn&& fn)
	{
		if (!bus_)
		{
			return MAKE_ERROR("Internal EventBus was null");
		}

		return OnInputImpl(*bus_, src, entity_, std::forward<Fn>(fn));
	}


private:
	Entity entity_;
	EventBus2* bus_ = nullptr;
};
