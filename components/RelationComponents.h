#pragma once
#include "BaseComponent.h"
#include "../ecs/EntityT.h"
#include <set>
#include <unordered_set>

struct Parent : BaseComponent<Parent>
{
    Entity_t entityId = kInvalidEntity;
};

struct Children : BaseComponent<Children>
{
    std::set<Entity_t> childEntityIds;
};

template <typename T>
concept RelationalComponentType = (std::same_as<T, Parent> || std::same_as<T, Children>);