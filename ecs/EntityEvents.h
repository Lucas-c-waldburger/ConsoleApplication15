#pragma once
#include "Ecs.h"
#include "../events/EventBus2.h"
#include "../inputs/InputState.h"
#include "../core/FuncTraits.h"
#include "../events/data/EntityEventConcept.h"
#include "../components/util/ComponentValidPreds.h"
#include "../components/ScriptComponent.h"

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

	template <typename Fn>
	static void call_for_timer(Fn&& fn, Entity& e)
	{
		if (!e.HasComponents<ComponentTs...>())
		{
			return;
		}

		std::invoke(fn, e.GetComponent<ComponentTs>()...);
	}
	template <typename Fn>
	static void call_for_timer(Fn&& fn, const Entity& e)
	{
		if (!e.HasComponents<ComponentTs...>())
		{
			return;
		}

		std::invoke(fn, e.GetComponent<ComponentTs>()...);
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

template <typename Tup, typename Fn>
inline void ForwardTimerCallbackComponents(Fn&& fn, Entity& e)
{
	return detail::get_components_helper<Tup>::call_for_timer(fn, e);
}
template <typename Tup, typename Fn>
inline void ForwardTimerCallbackComponents(Fn&& fn, const Entity& e)
{
	return detail::get_components_helper<Tup>::call_for_timer(fn, e);
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

template <typename Fn>
inline constexpr bool valid_event_script_callback_base_sig_v =
	(fn_returns_void_v<Fn> && func_traits<Fn>::argCount > 0);

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

	//static constexpr bool with_event_data_only_script_v = func_traits<Fn>::argCount == 1 &&
	//	!std::same_as<std::remove_cvref_t<typename func_traits<resolved_fn>::template arg_at<0>>, Entity>);

	//static constexpr bool with_entity_only_script_v = func_traits<Fn>::argCount == 1 &&
	//	std::same_as<typename func_traits<resolved_fn>::template arg_at<0>, Entity&>);

	//static constexpr bool with_event_data_and_entity_script_v = (func_traits<Fn>::argCount == 2 &&
	//	!std::same_as<std::remove_cvref_t<typename func_traits<resolved_fn>::template arg_at<0>>, Entity> &&
	//	std::same_as<typename func_traits<resolved_fn>::template arg_at<1>, Entity&>)
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

//template <typename Fn>
//inline constexpr bool valid_event_script_callback_sig_v = (
//	valid_event_script_callback_base_sig_v<Fn> &&
//	ev_callback_sig<Fn>::with_event_data_only_script_v ||
//	ev_callback_sig<Fn>::with_entity_only_script_v ||
//	ev_callback_sig<Fn>::with_event_data_and_entity_script_v
//);

template <typename Fn> requires fn_returns_void_v<Fn>
struct timer_ev_callback_sig
{
	static constexpr bool with_no_args_v = func_traits<Fn>::argCount == 0;

	static constexpr bool with_entity_v = (func_traits<Fn>::argCount == 1 &&
		std::same_as<typename func_traits<Fn>::template arg_at<0>, Entity&>);

	static constexpr bool with_const_entity_v = (func_traits<Fn>::argCount == 1 &&
		std::same_as<typename func_traits<Fn>::template arg_at<0>, const Entity&>);

	static constexpr bool with_components_v = (func_traits<Fn>::argCount >= 1 &&
		no_args_are_entities_v<typename func_traits<Fn>::arg_types> &&
		all_args_refs_v<typename func_traits<Fn>::arg_types>);

	static constexpr bool with_const_components_v = (func_traits<Fn>::argCount >= 1 &&
		no_args_are_entities_v<typename func_traits<Fn>::arg_types> &&
		all_args_const_refs_v<typename func_traits<Fn>::arg_types>);
};

template <typename Fn>
inline constexpr bool valid_timer_event_callback_sig_v = (
	timer_ev_callback_sig<Fn>::with_no_args_v ||
	timer_ev_callback_sig<Fn>::with_entity_v ||
	timer_ev_callback_sig<Fn>::with_const_entity_v ||
	timer_ev_callback_sig<Fn>::with_components_v ||
	timer_ev_callback_sig<Fn>::with_const_components_v
);

template <SomeInputSourceEnum Src, SomeInputEvent EvT>
inline constexpr bool input_src_matches_input_event_v = 
	std::same_as<extract_input_event_src_type_t<EvT>, Src>;

class EntityEvents
{
public:
	struct FilterDef
	{
		struct RelevantEntity
		{
			Entity_t id = kInvalidEntity;
			bool capture = false;
		};

		using AuxiliaryFilter = bool(*)(const Entity&);

		RelevantEntity relevantEntity;
		SDL_JoystickID relevantJoystickId = -1;
	};

	EntityEvents() = default;
	EntityEvents(const Entity& e, EventBus* bus) : entity_(e), bus_(bus) {}

	template <typename Fn> requires valid_event_callback_sig_v<Fn>
	Result<Void> OnEvent(Fn&& fn, FilterDef filterDef = {});

	template <typename Src, typename Fn> requires valid_input_callback_sig_v<Src, Fn>
	Result<Void> OnInput(Src src, Fn&& fn, FilterDef filterDef = {});

	template <typename Src, typename Fn> requires valid_input_callback_sig_v<Src, Fn>
	Result<Void> OnInput(Src src, InputState state, Fn&& fn, FilterDef filterDef = {});

	template <typename Fn> requires valid_timer_event_callback_sig_v<Fn>
	Result<Void> MakeTimer(float durationSec, Fn&& fn, int numRepeats = 0);

	// scripts
	template <typename EvT>
	Result<Void> OnEventScript(std::string_view scriptCallable, FilterDef filterDef = {});

	template <SomeInputEvent EvT, typename Src> requires input_src_matches_input_event_v<Src, EvT>
	Result<Void> OnInputScript(Src src, std::string_view scriptCallable, FilterDef filterDef = {});

	template <SomeInputEvent EvT, typename Src> requires input_src_matches_input_event_v<Src, EvT>
	Result<Void> OnInputScript(Src src, InputState state, std::string_view scriptCallable, 
							   FilterDef filterDef = {});

private:
	template <typename EvT>
	static auto MakeEventScriptCallback(Entity& e, std::string_view scriptCallable,
										FilterDef&& filterDef);

	template <typename EvT>
	static auto MakeInputScriptCallback(Entity& e, InputState st, std::string_view scriptCallable,
										FilterDef&& filterDef);

	Entity entity_;
	EventBus* bus_ = nullptr;
};


inline void RemoveTimerChild(Entity& ch)
{
	if (ch.IsValid() && (!ch.HasComponent<Timer>() || 
		ch.GetComponent<Timer>().numRepeats == 0))
	{
		ch.Destroy();
	}
}

template <SomeInputEvent Ev>
inline constexpr bool InputStateMatches(const Ev& ev, InputState state)
{
	return static_cast<bool>(ev.input.state & state);
}

template <HasEntityParticipants Ev>
inline bool IsEntityParticipantInEvent(const Ev& event, 
									   const EntityEvents::FilterDef& filterDef)
{
	static constexpr auto entityMatchesOne =
	[]<size_t I>(const EntityEvents::FilterDef& def, const Ev& ev) {
		return def.relevantEntity.id == ev.entity<I>();
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
	assert(ev.joystickID > -1);

	if (filterDef.relevantJoystickId == -1)
	{
		return true;
	}

	if (e.GetID() == filterDef.relevantEntity.id)
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

	auto relevantE = ECS::GetEntityByID(filterDef.relevantEntity.id);
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

inline Entity GetRelevantEntity(const Entity& e, const EntityEvents::FilterDef& def)
{
	return (def.relevantEntity.capture)
		? ECS::GetEntityByID(def.relevantEntity.id)
		: e;
}

template <typename Fn> requires valid_event_callback_sig_v<Fn>
inline void OnEventImpl(EventBus& bus, Entity& e, Fn&& fn, 
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
				auto relE = GetRelevantEntity(e, def);

				std::invoke(f, ev, relE);
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
				auto relE = GetRelevantEntity(e, def);

				ForwardEventCallbackComponents<cmps>(std::forward<Fn>(f), ev, relE);
			}
		}));
	}
}

template <typename Src, typename Fn> requires valid_input_callback_sig_v<Src, Fn>
inline void OnInputImpl(EventBus& bus, Src src, InputState state, Entity& e, Fn&& fn, 
						const EntityEvents::FilterDef& filterDef)
{
	using event_data_t =
		std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>;

	auto& tks = e.AddComponent<SignalTokenStorage>().signalTokens;

	if constexpr (ev_callback_sig<Fn>::with_event_data_only_v)
	{
		tks.emplace_back(bus.ConnectToInput(src,
		[e, st = state, def = filterDef, f = std::forward<Fn>(fn)](const event_data_t& ev) {
			if (IsEventRelevant(e, ev, def) && InputStateMatches(ev, st))
			{
				std::invoke(f, ev);
			}
		}));
	}
	else if constexpr (ev_callback_sig<Fn>::with_entity_v ||
					   ev_callback_sig<Fn>::with_const_entity_v)
	{
		tks.emplace_back(bus.ConnectToInput(src,
		[e, st = state, def = filterDef, f = std::forward<Fn>(fn)](const event_data_t& ev) mutable {
			if (IsEventRelevant(e, ev, def) && InputStateMatches(ev, st))
			{
				auto relE = GetRelevantEntity(e, def);

				std::invoke(f, ev, relE);
			}
		}));
	}
	else if constexpr (ev_callback_sig<Fn>::with_components_v ||
					   ev_callback_sig<Fn>::with_const_components_v)
	{
		using cmps = pop_front_t<remove_cvrefs_t<typename func_traits<Fn>::arg_types>>;

		tks.emplace_back(bus.ConnectToInput(src,
		[e, st = state, def = filterDef, f = std::forward<Fn>(fn)](const event_data_t& ev) mutable {
			if (IsEventRelevant(e, ev, def) && InputStateMatches(ev, st))
			{
				auto relE = GetRelevantEntity(e, def);

				ForwardEventCallbackComponents<cmps>(std::forward<Fn>(f), ev, relE);
			}
		}));
	}
}

template <typename Fn> requires valid_timer_event_callback_sig_v<Fn>
inline void MakeTimerImpl(EventBus& bus, Entity& e, Entity& ch, Fn&& fn)
{
	auto& tks = ch.AddComponent<SignalTokenStorage>().signalTokens;

	if constexpr (timer_ev_callback_sig<Fn>::with_no_args_v)
	{
		tks.emplace_back(bus.ConnectToEvent([e, ch, f = std::forward<Fn>(fn)]
		(const events::TimerFired& ev) mutable {
			if (ev.entity<0>() == ch.GetID())
			{
				std::invoke(f);
				RemoveTimerChild(ch);
			}
		}));
	}
	else if constexpr (timer_ev_callback_sig<Fn>::with_entity_v ||
					   timer_ev_callback_sig<Fn>::with_const_entity_v)
	{
		tks.emplace_back(bus.ConnectToEvent([e, ch, f = std::forward<Fn>(fn)]
		(const events::TimerFired& ev) mutable {
			if (ev.entity<0>() == ch.GetID())
			{
				std::invoke(f, e);
				RemoveTimerChild(ch);
			}
		}));
	}
	else if constexpr (timer_ev_callback_sig<Fn>::with_components_v ||
					   timer_ev_callback_sig<Fn>::with_const_components_v)
	{
		using cmps = remove_cvrefs_t<typename func_traits<Fn>::arg_types>;

		tks.emplace_back(bus.ConnectToEvent([e, ch, f = std::forward<Fn>(fn)]
		(const events::TimerFired& ev) mutable {
			if (ev.entity<0>() == ch.GetID())
			{
				ForwardTimerCallbackComponents<cmps>(std::forward<Fn>(f), e);
				RemoveTimerChild(ch);
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
	if (filterDef.relevantEntity.id == kInvalidEntity)
	{
		filterDef.relevantEntity.id = e.GetID();
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

	OnInputImpl(*bus_, src, InputState::AnyInput, entity_, std::forward<Fn>(fn), filterDef);

	return kVoid;
}

template <typename Src, typename Fn> requires valid_input_callback_sig_v<Src, Fn>
Result<Void> EntityEvents::OnInput(Src src, InputState state, Fn&& fn, FilterDef filterDef)
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

	OnInputImpl(*bus_, src, state, entity_, std::forward<Fn>(fn), filterDef);

	return kVoid;
}

template <typename Fn> requires valid_timer_event_callback_sig_v<Fn>
Result<Void> EntityEvents::MakeTimer(float durationSec, Fn&& fn, int numRepeats)
{
	if (!entity_.IsValid())
	{
		return MAKE_ERROR("Internal Entity was invalid");
	}
	if (!bus_)
	{
		return MAKE_ERROR("Internal EventBus was null");
	}

	auto rels = entity_.GetRelations();
	if (rels.IsChild())
	{
		return MAKE_ERROR("Internal Entity is a child - could not delegate new timer");
	}

	auto ch = rels.AddChild();
	ch.AddComponent(Timer{
		.duration = durationSec,
		.numRepeats = numRepeats
	});

	MakeTimerImpl(*bus_, entity_, ch, std::forward<Fn>(fn));

	return kVoid;
}

template <typename EvT>
inline auto ScriptCallbackImpl(Entity& e, const std::string& script, const EvT& ev)
{
	if (!e.HasComponent<Script>(&ScriptValid))
	{
		return;
	}

	auto func = e.GetComponent<Script>().table[script];
	if (!func.IsValid())
	{
		return;
	}

	sol::protected_function_result result;
	if (func.MatchesArguments<Entity&>())
	{
		result = func(e);
	}
	else if (func.MatchesArguments<const EvT&, Entity&>())
	{
		result = func(e, ev);
	}
	else if (func.MatchesArguments<Entity&, const EvT&>())
	{
		result = func(ev, e);
	}
	else
	{
		LOG_ERROR("Script function '{}' has invalid signature for event callback", script);
	}

	if (!result.valid())
	{
		sol::error err = result;
		LOG_ERROR_FMT("Error calling script function '{}': {}", script, err.what());
	}
}

template <typename EvT>
inline auto EntityEvents::MakeEventScriptCallback(Entity& e, std::string_view scriptCallable, 
												  FilterDef&& filterDef)
{
	return [e, script = std::string{ scriptCallable }, def = std::move(filterDef)](const EvT& ev) {
		if (!IsEventRelevant(e, ev, def))
		{
			return;
		}
		
		ScriptCallbackImpl(e, script, ev);
	};
}

template <typename EvT>
inline auto EntityEvents::MakeInputScriptCallback(Entity& e, InputState st, 
												  std::string_view scriptCallable, FilterDef&& filterDef)
{
	return [e, st, script = std::string{ scriptCallable }, def = std::move(filterDef)](const EvT& ev) {
		if (!(InputStateMatches(ev, st) && IsEventRelevant(e, ev, def)))
		{
			return;
		}
		
		ScriptCallbackImpl(e, script, ev);
	};
}

template <typename EvT>
inline Result<Void> EntityEvents::OnEventScript(std::string_view scriptCallable, FilterDef filterDef)
{
	if (!entity_.IsValid())
	{
		return MAKE_ERROR("Internal Entity was invalid");
	}
	if (!bus_)
	{
		return MAKE_ERROR("Internal EventBus was null");
	}

	auto& tks = entity_.AddComponent<SignalTokenStorage>().signalTokens;

	auto& tk = tks.emplace_back(bus_->ConnectToEvent(
		MakeEventScriptCallback<EvT>(entity_, scriptCallable, std::move(filterDef)))
	);
	tk.type = EntityCallbackToken::Type::Script;

	return kVoid;
}

template <SomeInputEvent EvT, typename Src> requires input_src_matches_input_event_v<Src, EvT>
inline Result<Void> EntityEvents::OnInputScript(Src src, InputState state, std::string_view scriptCallable,
												FilterDef filterDef)
{
	if (!entity_.IsValid())
	{
		return MAKE_ERROR("Internal Entity was invalid");
	}
	if (!bus_)
	{
		return MAKE_ERROR("Internal EventBus was null");
	}

	auto& tks = entity_.AddComponent<SignalTokenStorage>().signalTokens;

	auto& tk = tks.emplace_back(bus_->ConnectToInput(src,
		MakeInputScriptCallback<EvT>(entity_, state, scriptCallable, std::move(filterDef)))
	);
	tk.type = EntityCallbackToken::Type::Script;

	return kVoid;
} 

template <SomeInputEvent EvT, typename Src> requires input_src_matches_input_event_v<Src, EvT>
inline Result<Void> EntityEvents::OnInputScript(Src src, std::string_view scriptCallable, 
												FilterDef filterDef)
{
	return OnInputScript<EvT>(src, InputState::AnyInput, scriptCallable, std::move(filterDef));
}




