#pragma once
#include "BaseComponent.h"
#include "../ecs/EntityT.h"
#include "ComponentConcepts.h"
#include <unordered_map>

struct Parent : BaseComponent<Parent, 4>
{
    Entity_t entityId = kInvalidEntity;
};

struct Children : BaseComponent<Children, 5>
{
    std::unordered_set<Entity_t> childEntityIds;
};

template <typename T>
concept RelationalComponentType = (std::same_as<T, Parent> || std::same_as<T, Children>);