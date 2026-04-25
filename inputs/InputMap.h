#pragma once
#include "InputSource.h"
#include "InputField.h"
#include "../core/SizedEnumMap.h"

template <SomeInputSourceEnum Src, SomeInputField Val> requires
	std::same_as<Src, typename Val::SourceType>
using InputMap = SizedEnumMap<Src, Val>;

namespace detail {

template <typename InpMap>
struct make_input_map_impl;

template <typename Src, typename Val>
struct make_input_map_impl<InputMap<Src, Val>>
{
	static constexpr InputMap<Src, Val> call()
	{
		InputMap<Src, Val> inputMap{};

		for (size_t i = enum_start_v<Src>; i < enum_size_v<Src>; ++i)
		{
			auto src = static_cast<Src>(i);
			inputMap[src].source = src;
			inputMap[src].state = InputState::None;
		}

		return inputMap;
	}
};

} // detail


template <typename InpMap>
inline constexpr InpMap MakeInputMap()
{
	return detail::make_input_map_impl<InpMap>::call();
}