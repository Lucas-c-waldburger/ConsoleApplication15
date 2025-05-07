#pragma once
#include "../ComponentConcepts.h"

template <ComponentType T>
class ComponentBuilder;

namespace detail {

template <typename T>
struct is_component_builder : std::false_type {};

template <typename T>
struct is_component_builder<ComponentBuilder<T>> : std::true_type {};

} // detail

template <typename T>
concept SomeComponentBuilder = detail::is_component_builder<T>::value;