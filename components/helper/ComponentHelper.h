#pragma once
#include "../ComponentConcepts.h"

template <ComponentType T>
class ComponentHelper;

namespace detail {

	template <typename T>
	struct is_component_helper : std::false_type {};

	template <typename T>
	struct is_component_helper<ComponentHelper<T>> : std::true_type {};

} // detail

template <typename T>
concept SomeComponentHelper = detail::is_component_helper<T>::value;