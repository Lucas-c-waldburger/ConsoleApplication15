#pragma once
//#include "../../components/BaseComponent.h"
//#include "Weapon.h"
//#include "Item.h"
//#include "../../ecs/Ecs.h"
//#include "../../test/callbacks/GameControllerCallbacks.h"
//#include "../../events/EventBus2.h"
//#include "../../core/FuncTraits.h"

namespace game {

//namespace detail {
//
//template <typename Tup>
//struct no_args_are_entities;
//
//template <template <typename...> class Tup, typename...Ts>
//struct no_args_are_entities<Tup<Ts...>> : 
//	std::bool_constant<((!std::same_as<std::remove_cvref_t<Ts>, Entity>) && ...)> {};
//
//template <typename Tup>
//struct all_args_refs;
//
//template <template <typename...> class Tup, typename...Ts>
//struct all_args_refs<Tup<Ts...>> :
//	std::bool_constant<(is_non_const_reference_v<Ts> && ...)> {};
//
//template <typename Tup>
//struct all_args_const_refs;
//
//template <template <typename...> class Tup, typename...Ts>
//struct all_args_const_refs<Tup<Ts...>> :
//	std::bool_constant<(is_const_reference_v<Ts> && ...)> {};
//
//template <typename Tup> struct get_components_helper;
//
//template <template <typename...> class Tup, typename...Ts>
//struct get_components_helper<Tup<Ts...>>
//{
//	template <SomeEventData Ev, typename Fn>
//	static void call(Fn&& fn, const Ev& ev, Entity& e) 
//	{
//		if (!e.HasComponents<Ts...>())
//		{
//			return;
//		}
//
//		std::invoke(fn, ev, e.GetComponent<Ts>()...);
//	}
//	template <SomeEventData Ev, typename Fn>
//	static void call(Fn&& fn, const Ev& ev, const Entity& e)
//	{
//		if (!e.HasComponents<Ts...>())
//		{
//			return;
//		}
//
//		std::invoke(fn, ev, e.GetComponent<Ts>()...);
//	}
//};
//
//} // detail
//
//template <typename Tup>
//static constexpr bool no_args_are_entities_v = detail::no_args_are_entities<Tup>::value;
//
//template <typename Tup>
//static constexpr bool all_args_refs_v = detail::all_args_refs<Tup>::value;
//
//template <typename Tup>
//static constexpr bool all_args_const_refs_v = detail::all_args_const_refs<Tup>::value;
//
//template <typename Tup, typename Fn, SomeEventData Ev>
//inline void ForwardEventCallbackComponents(Fn&& fn, const Ev& ev, Entity& e)
//{
//	return detail::get_components_helper<Tup>::call(fn, ev, e);
//}
//template <typename Tup, typename Fn, SomeEventData Ev>
//inline void ForwardEventCallbackComponents(Fn&& fn, const Ev& ev, const Entity& e)
//{
//	return detail::get_components_helper<Tup>::call(fn, ev, e);
//}
//
//template <typename Fn> requires (func_traits<Fn>::argCount >= 1)
//struct event_callback_sig
//{
//	static constexpr bool returns_void = 
//		std::same_as<typename func_traits<Fn>::return_type, void>;
//
//	using args = typename func_traits<Fn>::arg_types;
//	static constexpr size_t arg_count = func_traits<Fn>::argCount;
//	template <size_t I> using arg_at = typename func_traits<Fn>::arg_at;
//
//	using arg_0 = arg_at<0>;
//	using arg_0_raw = std::remove_cvref_t<arg_0>;
//	static constexpr bool arg_0_is_event_data = (SomeEventData<arg_0_raw> &&
//												 is_const_reference_v<arg_0>);
//
//	static constexpr bool valid_base_sig = (returns_void && arg_0_is_event_data);
//
//	/** sig of (const SomeEventData&) */
//	template <typename = void> requires (valid_base_sig && arg_count == 1)
//	struct with_event_data_only : std::true_type {};
//
//	template <typename = void> requires (valid_base_sig && arg_count == 2)
//	struct with_entity
//	{
//		/** sig of (const SomeEventData&, Entity&) */
//		static constexpr bool ref = std::same_as<arg_at<1>, Entity&>;
//		/** sig of (const SomeEventData&, const Entity&) */
//		static constexpr bool const_ref = std::same_as<arg_at<1>, const Entity&>;
//	};
//
//	template <typename = void> requires (valid_base_sig && arg_count >= 2)
//	struct with_components
//	{
//		using component_args = pop_front_t<args>;
//
//		/** sig of (const SomeEventData&, SomeComponent&, ...) */
//		static constexpr bool ref = (no_args_are_entities_v<component_args> &&
//									 all_args_refs_v<component_args>);
//
//		/** sig of (const SomeEventData&, const SomeComponent&, ...) */
//		static constexpr bool const_ref = (no_args_are_entities_v<component_args> &&
//										   all_args_const_refs_v<component_args>);
//	};
//
//	static constexpr bool with_event_data_only_v = with_event_data_only::value;
//	static constexpr bool with_entity_v = with_entity::ref;
//	static constexpr bool with_const_entity_v = with_entity::const_ref;
//	static constexpr bool with_components_v = with_components::ref;
//	static constexpr bool with_const_components_v = with_components::const_ref;
//
//	static constexpr bool valid = (with_event_data_only_v || with_entity_v ||
//								   with_const_entity_v || with_components_v || 
//								   with_const_components_v);
//
//	static constexpr bool const_qualified = (with_event_data_only_v ||
//											 with_const_entity_v ||
//											 with_const_components_v);
//	static constexpr bool non_const_qualified = (with_event_data_only_v ||
//												 with_entity_v ||
//												 with_components_v);
//
//	template <typename = void> requires arg_0_is_event_data
//	using event_data_t = arg_0_raw;
//
//	using make_index_sequence_1_to_arg_count =
//		make_index_sequence_offset<1, arg_count>;
//};
//
//
//template <typename Src, typename Fn> 
//	requires (event_callback_sig<Fn>::valid && 
//			  SomeInputEvent<typename event_callback_sig<Fn>::event_data_t>)
//struct input_callback_sig
//{
//	using event_data_t = typename event_callback_sig<Fn>::event_data_t;
//	using extracted_src = extract_input_event_src_type_t<event_data_t>;
//	
//	static constexpr bool valid = std::same_as<Src, extracted_src>;
//};
//
//template <SomeEntityEvent Ev>
//inline bool IsEntityInvolvedInEvent(const Entity& entity, const Ev& event)
//{
//	static constexpr auto entityMatchesOne =
//	[]<size_t I>(const Entity & e, const Ev & ev) {
//		return e.GetID() == ev.entity<I>();
//	};
//
//	static constexpr auto entityMatchesAny =
//	[]<size_t...Is>(const Entity & e, const Ev & ev, std::index_sequence<Is...>) {
//		return (entityMatchesOne<Is>(e, ev) || ...);
//	};
//
//	return entityMatchesAny(entity, event, std::make_index_sequence<Ev::entityCount>{});
//}
//
//template <typename Fn> requires event_callback_sig<Fn>::valid
//static Result<Void> OnEventImpl(EventBus2& bus, Entity& e, Fn&& fn)
//{
//	if (!e.IsValid())
//	{
//		return MAKE_ERROR("Entity was invalid");
//	}
//
//	using sig = event_callback_sig<Fn>;
//	using event_data_t = typename sig::event_data_t;
//
//	auto& tks = e.GetComponent<SignalTokenStorage>().signalTokens;
//
//	if constexpr (sig::with_event_data_only_v)
//	{
//		//tks.emplace_back(bus.ConnectToEvent<event_data_t>(std::forward<Fn>(fn)));
//		tks.emplace_back(bus.ConnectToEvent<event_data_t>(
//			[e, &fn](const event_data_t& ev) {
//				if (!e.IsValid())
//				{
//					return;
//				}
//				if constexpr (SomeEntityEvent<event_data_t>)
//				{
//					if (!IsEntityInvolvedInEvent(e, ev))
//					{
//						return;
//					}
//				}
//
//				std::invoke(std::forward<Fn>(fn), ev);
//			}));
//
//		return Void{};
//	}
//	else if constexpr (sig::with_entity_v || sig::with_const_entity_v)
//	{
//		tks.emplace_back(bus.ConnectToEvent<event_data_t>(
//			[e, &fn](const event_data_t& ev) mutable {
//				if (!e.IsValid())
//				{
//					return;
//				}
//				if constexpr (SomeEntityEvent<event_data_t>)
//				{
//					if (!IsEntityInvolvedInEvent(e, ev))
//					{
//						return;
//					}
//				}
//
//				std::invoke(std::forward<Fn>(fn), ev, e);
//			}));
//
//		return Void{};
//	}
//	else if constexpr (sig::with_components_v || sig::with_const_components_v)
//	{
//		using cmps = typename sig::with_components::component_args;
//
//		tks.emplace_back(bus.ConnectToEvent<event_data_t>(
//			[e, &fn](const event_data_t& ev) mutable {
//				if (!e.IsValid())
//				{
//					return;
//				}
//				if constexpr (SomeEntityEvent<event_data_t>)
//				{
//					if (!IsEntityInvolvedInEvent(e, ev))
//					{
//						return;
//					}
//				}
//
//				ForwardEventCallbackComponents<cmps>(std::forward<Fn>(fn), ev, e);
//			}));	
//
//		return Void{};
//	}
//	else
//	{
//		return MAKE_ERROR("Internal inconsistency - event callback signature "
//			"did not conform to any valid signatures");
//	}
//}
//
//template <typename Src, typename Fn> requires input_callback_sig<Src, Fn>::valid
//static Result<Void> OnInputImpl(EventBus2& bus, Src src, Entity& e, Fn&& fn)
//{
//	using sig = event_callback_sig<Fn>;
//	using event_data_t = typename sig::event_data_t;
//
//	if (!e.IsValid())
//	{
//		return MAKE_ERROR("Entity was invalid");
//	}
//
//	auto& tks = e.GetComponent<SignalTokenStorage>().signalTokens;
//
//	if constexpr (sig::with_event_data_only_v)
//	{
//		tks.emplace_back(bus.ConnectToInput(src, std::forward<Fn>(fn)));
//
//		return Void{};
//	}
//	else if constexpr (sig::with_entity_v || sig::with_const_entity_v)
//	{
//		tks.emplace_back(bus.ConnectToInput(src,
//			[e, &fn](const event_data_t& ev) mutable {
//				std::invoke(std::forward<Fn>(fn), ev, e);
//			}));
//
//		return Void{};
//	}
//	else if constexpr (sig::with_components_v || sig::with_const_components_v)
//	{
//		using cmps = typename sig::with_components::component_args;
//
//		tks.emplace_back(bus.ConnectToInput(src,
//			[e, &fn](const event_data_t& ev) mutable {
//				ForwardEventCallbackComponents<cmps>(std::forward<Fn>(fn), ev, e);
//			}));
//		 
//		return Void{};
//	}
//	else
//	{
//		return MAKE_ERROR("Internal inconsistency - input callback signature "
//			"did not conform to any valid signatures");
//	}
//}
//
//
//
//class EntityEvents : private Entity
//{
//public:
//	EntityEvents() = default;
//
//	template <typename Fn> requires event_callback_sig<Fn>::valid
//	Result<Void> OnEvent(Fn&& fn)
//	{
//		if (!bus_)
//		{
//			return MAKE_ERROR("Internal EventBus was null");
//		}
//		return OnEventImpl(*bus_, *this, std::forward<Fn>(fn));
//	}
//
//	template <typename Src, typename Fn> requires input_callback_sig<Src, Fn>::valid
//	Result<Void> OnInput(Src src, Fn&& fn)
//	{
//		if (!bus_)
//		{
//			return MAKE_ERROR("Internal EventBus was null");
//		}
//		return OnInputImpl(*bus_, src, *this, std::forward<Fn>(fn));
//	}
//
//protected:
//	EntityEvents(const Entity& e, EventBus2* bus) : Entity(e), bus_(bus) {}
//
//private:
//	EventBus2* bus_ = nullptr;
//};

//struct PlayerState
//{
//	enum : uint8_t
//	{
//		Moving = 1 << 0,
//		Airborne = 1 << 1
//	};
//	uint8_t value = 0;
//};
//
//class ControllerEventHandler
//{
//public:
//	ControllerEventHandler(Entity& e, EventBus2& bus) : entity_(e), bus_(bus)
//	{
//		auto& tks = entity_.AddComponent<SignalTokenStorage>().signalTokens;
//		tks.emplace_back(bus_.ConnectToEvent(test::ConnectToFirstController(entity_)));
//		tks.emplace_back(bus_.ConnectToEvent(test::DisconnectController(entity_)));
//	}
//
//	using Src = GameControllerInputSource;
//
//	enum PredicateMoniker
//	{
//		None,
//		And,
//		Or,
//		Not
//	};
//
//	enum PredicateType
//	{
//		State,
//		EntityCondition
//	};
//
//	struct StatePredicate
//	{
//		InputState state;
//		std::optional<float> duration;
//	};
//
//	struct Composer
//	{
//		Composer(Entity e, Src src) : entity(e), source(src) {}
//
//		Composer& If(StatePredicate statePred)
//		{
//			statePredicates.emplace_back(std::move(statePred));
//			predicateTypeChain.emplace_back(PredicateType::State);
//			return *this;
//		}
//		Composer& Or(StatePredicate statePred)
//		{
//			statePredicates.emplace_back(std::move(statePred));
//			predicateTypeChain.emplace_back(PredicateType::State);
//			monikers.emplace_back(PredicateMoniker::Or);
//			return *this;
//		}
//
//		template <typename Fn>
//		Composer& If(Fn&& fn)
//		{
//			if (!entity.IsValid()) { return *this; }
//
//			using components = typename func_traits<Fn>::arg_types;
//			  
//
//		}
//		
//		//template <typename Fn> requires std::same_as<std::invoke_result_t<Fn>, bool>
//		//Composer& If(Fn&& fn)
//
//		template <typename Fn> requires std::invocable<Fn, Entity&>
//		Composer& Do(Fn&& fn)
//		{
//			auto& tks = entity_.AddComponent<SignalTokenStorage>().signalTokens;
//			tks.emplace_back(bus_.ConnectToInput(source,
//				[e = entity, f = std::forward<Fn>(fn), source, state](const auto& ev) {
//					std::invoke(f, e, ev);
//			}));
//		}
//
//		Src source;
//		InputState state;
//		Entity entity;
//
//		std::vector<PredicateType> predicateTypeChain;
//		std::vector<PredicateMoniker> monikers;
//		std::vector<StatePredicate> statePredicates;
//		std::vector<fu2::unique_function<void()>> componentPredicates;
//
//
//		std::vector<std::pair<InputState, fu2::unique_function<void()>>> predicates;
//	};
//
//	Composer OnInput(Src src)
//	{
//		return Composer{ src };
//	}
//
//	//struct StateTemp
//	//{
//
//	//	InputState state;
//	//};
//
//private:
//	auto MakeStatePredicate(InputState state, std::optional<float> duration)
//	{
//		return [state, duration](const events::GameControllerInput& ev) {
//			return ev.input.
//		};
//	}
//
//	template <typename Fn>
//	void OnInputImpl(Src src, Fn&& fn)
//	{
//		auto& tks = entity_.AddComponent<SignalTokenStorage>().signalTokens;
//		tks.emplace_back(bus_.ConnectToInput(src, 
//			[e = entity_, f = std::forward<Fn>(fn)](const auto& ev) {
//				std::invoke(f, e, ev);
//		}));
//	}
//
//
//
//	Entity entity_;
//	EventBus2& bus_;
//};

//template <typename Derived, typename ValueType>
//struct ValueComponent;
//
//template <typename Derived, typename ValueType>
//	requires std::is_default_constructible_v<ValueType>
//struct ValueComponent<Derived, ValueType> : BaseComponent<Derived>
//{
//	ValueType value{};
//};
//
//
//template <typename Derived, typename BaseValueType = float>
//	requires std::is_default_constructible_v<BaseValueType>
//struct StatComponent : BaseComponent<Derived>
//{
//	BaseValueType base{};
//	float scale = 1.0f;
//};
//
//// STATS
//struct MovementSpeed : StatComponent<MovementSpeed> {};
//struct JumpHeight : StatComponent<JumpHeight> {};
//struct ExtraJumps : ValueComponent<ExtraJumps, int> {};
//struct Size : StatComponent<Size> {};
//struct AttackSpeed : StatComponent<AttackSpeed> {};
//
//
//// STATE
//struct PlayerState : BaseComponent<PlayerState>
//{
//	enum 
//	{
//		Grounded,
//		Airborne
//	} state;
//};
//
//struct PlayerWeapons : BaseComponent<PlayerWeapons>
//{
//	std::array<WeaponID, 4> weaponIds = {
//		kInvalidWeaponID, kInvalidWeaponID, 
//		kInvalidWeaponID, kInvalidWeaponID 
//	};
//	size_t count = 0;
//};
//
//struct PlayerItems : BaseComponent<PlayerItems>
//{
//	std::array<ItemID, 4> itemIds = {
//		kInvalidItemID, kInvalidItemID,
//		kInvalidItemID, kInvalidItemID
//	};
//	size_t count = 0;
//};

} // game