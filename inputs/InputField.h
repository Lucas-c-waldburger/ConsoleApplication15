#pragma once
#include "InputSource.h"
#include "InputState.h"

template <SomeInputSourceEnum T, typename U>
struct InputField
{
	using SourceType = T;
	using FieldValueType = U;

	SourceType source = static_cast<SourceType>(-1);
	InputState state = InputState::None;
	uint32_t stateDuration = 0;
	FieldValueType value;
};

namespace detail {

template <typename T>
struct is_input_field : std::false_type {};

template <typename Src, typename Val>
struct is_input_field<InputField<Src, Val>> : std::true_type {};

} // detail

template <typename T>
concept SomeInputField = detail::is_input_field<T>::value;