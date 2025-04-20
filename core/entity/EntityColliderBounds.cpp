#include "EntityColliderBounds.h"
#include "../../ecs/Ecs.h"

std::optional<EntityColliderBounds> EntityColliderBounds::CreateFromEntity(const Entity& entity, uint8_t filter)
{
    if (!entity.IsValid())
    {
        LOG_WARNING("Entity was invalid");
        return std::nullopt;
    }
    if (!entity.HasComponent<Collider>())
    {
        LOG_WARNING("Entity did not have collider component");
        return std::nullopt;
    }

    auto& collider = entity.GetComponent<Collider>();

    if ((collider.profile & filter) != filter)
    {
        return std::nullopt;
    }

    float width = collider.dimensions.w;
    float height = collider.dimensions.h;

    if (entity.HasComponent<Transform>() &&
        (collider.profile & Collider::Profile::ApplyScale))
    {
        const auto& tf = entity.GetComponent<Transform>();

        width *= tf.scale.x;
        height *= tf.scale.y;
    }

    AABB aabb{
        collider.position.x - (width / 2.0f),
        collider.position.y - (height / 2.0f),
        width,
        height
    };

    return EntityColliderBounds{ .entityId = entity.GetID(), .boundingBox = aabb };
}
