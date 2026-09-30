#pragma once
#include "ComponentConcepts.h"
#include "../ecs/EntityT.h"
#include <set>
#include <unordered_set>

struct Parent
{
    Entity_t entityId = kInvalidEntity;

    constexpr bool operator==(const Parent&) const = default;
};

struct Children
{
    std::set<Entity_t> childEntityIds;

    bool operator==(const Children&) const = default;
};

template <typename T>
concept RelationalComponentType = (std::same_as<T, Parent> || std::same_as<T, Children>);