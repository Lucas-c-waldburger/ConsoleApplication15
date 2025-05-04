#pragma once
#include <concepts>

class System {};

template <typename T>
concept SomeSystem = std::derived_from<T, System>;