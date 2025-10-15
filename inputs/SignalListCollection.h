#pragma once
#include "../events/data/EventDataIncludes.h"
#include "InputSignalList.h"

using InputSignalLists = std::tuple<
	InputSignalList<events::GameControllerInput>,
	InputSignalList<events::MouseInput>
>;


namespace detail {
template <SomeInputSourceEnum Source, typename InpSigLists>
struct index_of_input_signal_list_for_src;

template <SomeInputSourceEnum Source, template <typename...> class InpSigLists>
struct index_of_input_signal_list_for_src<Source, InpSigLists<>>
	: std::integral_constant<size_t, static_cast<size_t>(-1)> {};

template <SomeInputSourceEnum Source, template <typename...> class InpSigLists, typename T, typename... Ts>
struct index_of_input_signal_list_for_src<Source, InpSigLists<T, Ts...>>
	: std::conditional_t<
		std::is_same_v<Source, typename T::InputSourceType>,
		std::integral_constant<size_t, 0>,
		std::conditional_t<
			(sizeof...(Ts) > 0),
			std::integral_constant<size_t, 1 + index_of_input_signal_list_for_src<Source, InpSigLists<Ts...>>::value>,
			std::integral_constant<size_t, static_cast<size_t>(-1)>
		>
	> {};
} // namespace detail

template <SomeInputSourceEnum Source>
inline constexpr size_t index_of_input_signal_list_for_src_v =
detail::index_of_input_signal_list_for_src<Source, InputSignalLists>::value;


struct SignalListCollection
{
	EventSignalList eventSignals;
	InputSignalLists inputSignals;

	template <SomeInputSourceEnum Source>
	decltype(auto) GetInputSignalList()
	{
		static constexpr size_t idx = index_of_input_signal_list_for_src_v<Source>;
		static_assert(idx != static_cast<size_t>(-1), "Source not mappped to an input signal list");

		return std::get<idx>(inputSignals);
	}

	template <SomeEventData T>
	void Emit(const T& ev)
	{
		eventSignals.Emit(ev);

		if constexpr (type_in_tuple_v<InputSignalList<T>, InputSignalLists>)
		{
			std::get<InputSignalList<T>>(inputSignals).Emit(ev);
		}
	}
};