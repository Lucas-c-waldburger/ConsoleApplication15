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
		if (!e.HasComponents<ComponentTs...>())
		{
			return;
		}

		std::invoke(fn, ev, e.GetComponent<ComponentTs>()...);
	}
	template <typename Ev, typename Fn>
	static void call(Fn&& fn, const Ev& ev, const Entity& e)
	{
		if (!e.HasComponents<ComponentTs...>())
		{
			return;
		}

		std::invoke(fn, ev, e.GetComponent<ComponentTs>()...);
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

template <typename Src, typename Fn>
inline constexpr bool valid_input_callback_sig_v = (
	valid_event_callback_sig_v<Fn> && 
	SomeInputEvent<std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>> &&
	std::same_as<extract_input_event_src_type_t<
		std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>>, Src>
);

class EntityEvents
{
public:
	struct FilterDef
	{
		using AuxiliaryFilter = bool(*)(const Entity&);

		Entity_t relevantEntity = kInvalidEntity;
		SDL_JoystickID relevantJoystickId = -1;
	};

	EntityEvents() = default;
	EntityEvents(const Entity& e, EventBus2* bus) : entity_(e), bus_(bus) {}

	template <typename Fn> requires valid_event_callback_sig_v<Fn>
	Result<Void> OnEvent(Fn&& fn, FilterDef filterDef = {});

	template <typename Src, typename Fn> requires valid_input_callback_sig_v<Src, Fn>
	Result<Void> OnInput(Src src, Fn&& fn, FilterDef filterDef = {});

	//template <typename T>
	//bool ShouldProduceEvent() const;

private:
	Entity entity_;
	EventBus2* bus_ = nullptr;
};

template <HasEntityParticipants Ev>
inline bool IsEntityParticipantInEvent(const Ev& event, 
									const EntityEvents::FilterDef& filterDef)
{
	static constexpr auto entityMatchesOne =
	[]<size_t I>(const EntityEvents::FilterDef& def, const Ev& ev) {
		return def.relevantEntity == ev.entity<I>();
	};

	static constexpr auto entityMatchesAny =
	[]<size_t...Is>(const EntityEvents::FilterDef& def, const Ev& ev, 
					std::index_sequence<Is...>) {
		return (entityMatchesOne.template operator()<Is>(def, ev) || ...);
	};

	return entityMatchesAny(filterDef, event, 
							std::make_index_sequence<Ev::entityCount>{});
}

template <SomeGameControllerEvent Ev>
inline bool IsGameControllerInEvent(const Entity& e, const Ev& ev,
									const EntityEvents::FilterDef& filterDef)
{
	//return true;
	
	assert(ev.joystickID > -1);

	if (e.GetID() == filterDef.relevantEntity)
	{
		if (!e.HasComponent<GameControllerState>())
		{
			return false;
		}

		if constexpr (std::same_as<Ev, events::GameControllerConnected>)
		{
			// we're trying to get a valid joystick id
			return true;
		}

		const auto& gc = e.GetComponent<GameControllerState>();

		if (gc.joystickID == ev.joystickID)
		{
			return true;
		}

		return false;
	}

	auto relevantE = ECS::GetEntityByID(filterDef.relevantEntity);
	if (!relevantE.IsValid())
	{
		return false;
	}

	return relevantE.HasComponent<GameControllerState>() &&
		   relevantE.GetComponent<GameControllerState>()
			.joystickID == ev.joystickID;
}

template <typename Ev>
inline bool IsEventRelevant(const Entity& e, const Ev& ev, 
						    const EntityEvents::FilterDef& filterDef)
{
	if (!e.IsValid())
	{
		return false;
	}

	if constexpr (HasEntityParticipants<Ev>)
	{
		if (!IsEntityParticipantInEvent(ev, filterDef))
		{
			return false;
		}
	}
	if constexpr (SomeGameControllerEvent<Ev>)
	{
		return IsGameControllerInEvent(e, ev, filterDef);
	}

	return true;	
}

template <typename Fn> requires valid_event_callback_sig_v<Fn>
inline void OnEventImpl(EventBus2& bus, Entity& e, Fn&& fn, 
						const EntityEvents::FilterDef& filterDef)
{
	using event_data_t = 
		std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>;

	auto& tks = e.AddComponent<SignalTokenStorage>().signalTokens;

	if constexpr (ev_callback_sig<Fn>::with_event_data_only_v)
	{
		tks.emplace_back(bus.ConnectToEvent(
		[e, def = filterDef, f = std::forward<Fn>(fn)](const event_data_t& ev) {
			if (IsEventRelevant(e, ev, def))
			{
				std::invoke(f, ev);
			}
		}));
	}
	else if constexpr (ev_callback_sig<Fn>::with_entity_v || 
					   ev_callback_sig<Fn>::with_const_entity_v)
	{
		tks.emplace_back(bus.ConnectToEvent(
		[e, def = filterDef, f = std::forward<Fn>(fn)](const event_data_t& ev) mutable {
			if (IsEventRelevant(e, ev, def))
			{
				std::invoke(f, ev, e);
			}
		}));
	}
	else if constexpr (ev_callback_sig<Fn>::with_components_v || 
					   ev_callback_sig<Fn>::with_const_components_v)
	{
		using cmps = pop_front_t<remove_cvrefs_t<typename func_traits<Fn>::arg_types>>;

		tks.emplace_back(bus.ConnectToEvent(
		[e, def = filterDef, f = std::forward<Fn>(fn)](const event_data_t& ev) mutable {
			if (IsEventRelevant(e, ev, def))
			{
				ForwardEventCallbackComponents<cmps>(std::forward<Fn>(f), ev, e);
			}
		}));
	}
}

template <typename Src, typename Fn> requires valid_input_callback_sig_v<Src, Fn>
inline void OnInputImpl(EventBus2& bus, Src src, Entity& e, Fn&& fn, 
						const EntityEvents::FilterDef& filterDef)
{
	using event_data_t =
		std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>;

	auto& tks = e.AddComponent<SignalTokenStorage>().signalTokens;

	if constexpr (ev_callback_sig<Fn>::with_event_data_only_v)
	{
		tks.emplace_back(bus.ConnectToInput(src,
		[e, def = filterDef, f = std::forward<Fn>(fn)](const event_data_t& ev) {
			if (IsEventRelevant(e, ev, def))
			{
				std::invoke(f, ev);
			}
		}));
	}
	else if constexpr (ev_callback_sig<Fn>::with_entity_v ||
					   ev_callback_sig<Fn>::with_const_entity_v)
	{
		tks.emplace_back(bus.ConnectToInput(src,
		[e, def = filterDef, f = std::forward<Fn>(fn)](const event_data_t& ev) mutable {
			if (IsEventRelevant(e, ev, def))
			{
				std::invoke(f, ev, e);
			}
		}));
	}
	else if constexpr (ev_callback_sig<Fn>::with_components_v ||
					   ev_callback_sig<Fn>::with_const_components_v)
	{
		using cmps = pop_front_t<remove_cvrefs_t<typename func_traits<Fn>::arg_types>>;

		tks.emplace_back(bus.ConnectToInput(src,
		[e, def = filterDef, f = std::forward<Fn>(fn)](const event_data_t& ev) mutable {
			if (IsEventRelevant(e, ev, def))
			{
				ForwardEventCallbackComponents<cmps>(std::forward<Fn>(f), ev, e);
			}
		}));
	}
}

template <typename Fn>
inline void ResolveFilterDefinition(Entity& e, const Fn& fn, 
									EntityEvents::FilterDef& filterDef)
{
	using event_data_t =
		std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>;

	// try to match current entity if none specified
	if (filterDef.relevantEntity == kInvalidEntity)
	{
		filterDef.relevantEntity = e.GetID();
	}

	//if constexpr (SomeGameControllerEvent<event_data_t>)
	//{
	//	if (filterDef.relevantEntity != kInvalidEntity &&
	//		filterDef.relevantJoystickId == -1)
	//	{
	//		// entity specified, try to find its corresponding joystick id
	//		auto relevantEntity = ECS::GetEntityByID(filterDef.relevantEntity);

	//		if (relevantEntity.IsValid() &&
	//			relevantEntity.HasComponent<GameControllerState>())
	//		{
	//			filterDef.relevantJoystickId =
	//				relevantEntity.GetComponent<GameControllerState>().joystickID;
	//		}
	//	}
	//}
}

template <typename Fn> requires valid_event_callback_sig_v<Fn>
Result<Void> EntityEvents::OnEvent(Fn&& fn, FilterDef filterDef)
{
	if (!entity_.IsValid())
	{
		return MAKE_ERROR("Internal Entity was invalid");
	}
	if (!bus_)
	{
		return MAKE_ERROR("Internal EventBus was null");
	}

	ResolveFilterDefinition(entity_, fn, filterDef);

	OnEventImpl(*bus_, entity_, std::forward<Fn>(fn), filterDef);

	return kVoid;
}

template <typename Src, typename Fn> requires valid_input_callback_sig_v<Src, Fn>
Result<Void> EntityEvents::OnInput(Src src, Fn&& fn, FilterDef filterDef)
{
	if (!entity_.IsValid())
	{
		return MAKE_ERROR("Internal Entity was invalid");
	}
	if (!bus_)
	{
		return MAKE_ERROR("Internal EventBus was null");
	}

	ResolveFilterDefinition(entity_, fn, filterDef);

	OnInputImpl(*bus_, src, entity_, std::forward<Fn>(fn), filterDef);

	return kVoid;
}

//template <typename T>
//bool EntityEvents::ShouldProduceEvent() const
//{
//	if (!(entity_.IsValid() && bus_))
//	{
//		return false;
//	}
//
//
//}
