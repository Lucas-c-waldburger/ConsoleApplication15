#pragma once
#include "../core/FuncTraits.h"
#include "data/InputEventConcepts.h"

// type traits + constraints on what will be accepted by the "Connect()" methods on EventBus
// constraints are evaluated in a programmatic way via static_asserts for better error messages
template <typename Fn>
struct valid_signal_fn
{
	static_assert(HasFuncTraits<Fn>, 
		"type is not callable");
	static_assert(func_traits<Fn>::arg_types::size == 1, 
		"callable must have one argument");

	using ret_type = typename func_traits<Fn>::return_type;
	static_assert(std::same_as<ret_type, void>, 
		"callable must return void");

	using first_arg = type_at_index_t<0, typename func_traits<Fn>::arg_types>;
	static_assert(SomeEventData<raw_type_t<first_arg>>, 
		"argument must be an event data type");
	static_assert(is_const_reference_v<first_arg>, 
		"argument must be a const reference");

	using raw_event_data_type = raw_type_t<first_arg>;

	// defines a function with signature void(*)(const 'SomeEventData' &)
	struct event : std::true_type 
	{
		// explicitly makes sure the event data type of Fn matches T
		template <SomeEventData T>
		struct of_type 
		{
			static_assert(std::same_as<T, raw_event_data_type>,
				"callable's event argument does not match supplied event type");

			static constexpr bool value = true;
		};
	};

	// same sig as above, but 'SomeEventData' must meet the 'SomeInputEvent' concept,
	// and the Source template arg provided must be the same input source of that event
	template <SomeInputSourceEnum Source>
	struct input 
	{
		static_assert(SomeInputEvent<raw_event_data_type>, 
			"callable's event argument is not an input event");

		using input_src_type = extract_input_event_src_type_t<raw_event_data_type>;
		static_assert(std::same_as<input_src_type, Source>, 
			"callable's input event argument does not correspond to supplied source type");

		static constexpr bool value = true;

		// input with the check for an explicit Input Event type
		template <SomeInputEvent T>
		struct of_type
		{
			static_assert(std::same_as<T, raw_event_data_type>,
				"callable's input event argument does not match supplied input event type");

			static constexpr bool value = true;
		};
	};
};

// Use these externally
template <typename Fn>
concept ValidEventSignalFn = valid_signal_fn<Fn>::event::value;

template <typename Fn, typename T>
concept ValidEventSignalFnOfType = valid_signal_fn<Fn>::event::template of_type<T>::value;

template <typename Fn, typename Source>
concept ValidInputSignalFn = valid_signal_fn<Fn>::template input<Source>::value;

template <typename Fn, typename Source, typename T>
concept ValidInputSignalFnOfType = 
	valid_signal_fn<Fn>::template input<Source>::template of_type<T>::value;