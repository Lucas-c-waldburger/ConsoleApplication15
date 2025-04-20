#pragma once
#include <optional>
#include "../AABB.h"
#include "../../ecs/EntityT.h"

class Entity;

struct EntityColliderBounds
{
    Entity_t entityId;
    AABB boundingBox;

    constexpr bool operator==(const EntityColliderBounds& rhs) const {
        return entityId == rhs.entityId;
    }

    static std::optional<EntityColliderBounds> 
    CreateFromEntity(const Entity& entity, uint8_t filter = 0x00);
};

namespace std {
template <>
struct hash<EntityColliderBounds> {
    size_t operator()(const EntityColliderBounds& ecb) const noexcept {
        return std::hash<Entity_t>{}(ecb.entityId);
    }
};
}