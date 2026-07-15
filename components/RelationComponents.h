#pragma once
#include "BaseComponent.h"
#include "../ecs/EntityT.h"
#include <set>
#include <unordered_set>

struct Parent : BaseComponent<Parent>
{
    Entity_t entityId = kInvalidEntity;

    friend constexpr bool operator==(const Parent& lhs, const Parent& rhs) noexcept
    {
        return lhs.entityId == rhs.entityId;
    }
};

struct Children : BaseComponent<Children>
{
    std::set<Entity_t> childEntityIds;

    friend bool operator==(const Children& lhs, const Children& rhs)
    {
        return lhs.childEntityIds == rhs.childEntityIds;
    }
};

template <typename T>
concept RelationalComponentType = (std::same_as<T, Parent> || std::same_as<T, Children>);