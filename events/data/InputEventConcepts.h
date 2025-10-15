#pragma once
#include "../EventConcepts.h"
#include "../../inputs/InputField.h"

template <typename T>
concept SomeInputEvent = SomeEventData<T> && requires(T t) {
	SomeInputField<raw_type_t<decltype(t.input)>>;
};

namespace detail {

template <SomeInputEvent T>
struct extract_input_event_src_type
{
	using input_field_type = std::remove_cvref_t<decltype(std::declval<T>().input)>;
	using input_source_type = typename input_field_type::SourceType;
};

} // detail

template <SomeInputEvent T>
using extract_input_event_src_type_t = 
	detail::extract_input_event_src_type<T>::input_source_type;