#pragma once
#include <concepts>
#include "../ecs/EntityAccess.h"

class Entity;

class System : public EntityFullAccessPrivelage
{};

template <typename T>
concept SomeSystem = std::derived_from<T, System>;
