#pragma once
#include "../core/FuncTraits.h"
#include "data/InputEventConcepts.h"

//// type traits + constraints on what will be accepted by the "Connect()" methods on EventBus
//// constraints are evaluated in a programmatic way via static_asserts for better error messages
//template <typename Fn>
//struct valid_signal_fn
//{
//	static_assert(func_traits<Fn>::arg_types::size == 1, 
//		"callable must have one argument");
//
//	using ret_type = typename func_traits<Fn>::return_type;
//	static_assert(std::same_as<ret_type, void>, 
//		"callable must return void");
//
//	using first_arg = type_at_index_t<0, typename func_traits<Fn>::arg_types>;
//	static_assert(SomeEventData<raw_type_t<first_arg>>, 
//		"argument must be an event data type");
//	static_assert(is_const_reference_v<first_arg>, 
//		"argument must be a const reference");
//
//	using raw_event_data_type = raw_type_t<first_arg>;
//
//	// defines a function with signature void(*)(const 'SomeEventData' &)
//	struct event : std::true_type 
//	{
//		// explicitly makes sure the event data type of Fn matches T
//		template <SomeEventData T>
//		struct of_type 
//		{
//			static_assert(std::same_as<T, raw_event_data_type>,
//				"callable's event argument does not match supplied event type");
//
//			static constexpr bool value = true;
//		};
//	};
//
//	// same sig as above, but 'SomeEventData' must meet the 'SomeInputEvent' concept,
//	// and the Source template arg provided must be the same input source of that event
//	template <SomeInputSourceEnum Source>
//	struct input 
//	{
//		static_assert(SomeInputEvent<raw_event_data_type>, 
//			"callable's event argument is not an input event");
//
//		using input_src_type = extract_input_event_src_type_t<raw_event_data_type>;
//		static_assert(std::same_as<input_src_type, Source>, 
//			"callable's input event argument does not correspond to supplied source type");
//
//		static constexpr bool value = true;
//
//		// input with the check for an explicit Input Event type
//		template <SomeInputEvent T>
//		struct of_type
//		{
//			static_assert(std::same_as<T, raw_event_data_type>,
//				"callable's input event argument does not match supplied input event type");
//
//			static constexpr bool value = true;
//		};
//	};
//};
template <typename Fn>
	requires std::same_as<void, typename func_traits<Fn>::return_type>
struct base_valid_event_signal_fn
{
	struct Fake {};
	using Fallback = Fake(*)(Fake);

	using ResolvedFn = std::conditional_t<
		func_traits<Fn>::argCount == 1,
		Fn,
		Fallback
	>;

	static constexpr bool value = (
		is_const_reference_v<typename func_traits<ResolvedFn>::template arg_at<0>>
	);
};

template <typename Fn> requires base_valid_event_signal_fn<Fn>::value
struct valid_event_signal_fn
{
	static constexpr bool value = (
		SomeEventData<std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>>
	);
};

template <typename Fn, SomeInputSourceEnum Src> 
	requires (valid_event_signal_fn<Fn>::value &&
			  SomeInputEvent<std::remove_cvref_t<
				  typename func_traits<Fn>::template arg_at<0>>>)
struct valid_input_event_signal_fn
{
	using input_src_type = extract_input_event_src_type_t<
		std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>
	>;

	static constexpr bool value = std::same_as<input_src_type, Src>;
};

template <typename Fn> requires base_valid_event_signal_fn<Fn>::value
struct valid_user_event_signal_fn
{
	static constexpr bool value = !(
		SomeEventData<std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>>
	);
};

// Use these externally
template <typename Fn>
concept ValidEventSignalFn = valid_event_signal_fn<Fn>::value;

template <typename Fn>
concept ValidUserEventSignalFn = valid_user_event_signal_fn<Fn>::value;

template <typename Fn, typename T>
concept ValidEventSignalFnOfType = ValidEventSignalFn<Fn> &&
	std::same_as<std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>, T>;

template <typename Fn, typename Source>
concept ValidInputSignalFn = valid_input_event_signal_fn<Fn, Source>::value;

template <typename Fn, typename Source, typename T>
concept ValidInputSignalFnOfType = ValidInputSignalFn<Fn, Source>&&
	std::same_as<std::remove_cvref_t<typename func_traits<Fn>::template arg_at<0>>, T>;