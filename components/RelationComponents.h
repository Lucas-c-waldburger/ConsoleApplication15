#pragma once
#include "BaseComponent.h"
#include "../ecs/EntityT.h"
#include "ComponentConcepts.h"
#include <unordered_map>

//struct Parent : BaseComponent<Parent, 4>
struct Parent : BaseComponent<Parent>
{
    Entity_t entityId = kInvalidEntity;
};

//struct Children : BaseComponent<Children, 5>
struct Children : BaseComponent<Children>
{
    std::unordered_set<Entity_t> childEntityIds;
};

template <typename T>
concept RelationalComponentType = (std::same_as<T, Parent> || std::same_as<T, Children>);