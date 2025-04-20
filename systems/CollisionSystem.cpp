#include "CollisionSystem.h"
#include "../ecs/Ecs.h"
#include "../components/util/EventObserverUtils.h"
#include "../core/entity/EntityColliderBounds.h"
#include "../sdl/SDLite.h"
#include "../sdl/SDLUtils.h"

namespace {

// TODO: make these available outside
constexpr float kEpsilon = 0.001f;

float GetMagnitude(SDL_FPoint v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

SDL_FPoint Normalize(SDL_FPoint v) 
{
    float magnitude = GetMagnitude(v);
    if (magnitude > 0.00001f) 
    { 
        return { v.x / magnitude, v.y / magnitude }; 
    }
    else 
    {
        return { 0.0f, 0.0f };  
    }
}

constexpr float DotProduct(const SDL_FPoint& a, const SDL_FPoint& b) 
{
    return a.x * b.x + a.y * b.y;  
}
/* 
If DotProduct(a, b) > 0, the vectors are pointing roughly in the same direction.
If DotProduct(a, b) < 0, the vectors are pointing in opposite directions.
If DotProduct(a, b) == 0, the vectors are perpendicular (no projection of one vector onto the other).
*/

float CalculateOverlapArea(const AABB& rectA, const AABB& rectB) {
    // Calculate the width of the overlapping region
    float overlapWidth = std::max(0.0f, std::min(rectA.x + rectA.w, rectB.x + rectB.w) - 
        std::max(rectA.x, rectB.x));

    // Calculate the height of the overlapping region
    float overlapHeight = std::max(0.0f, std::min(rectA.y + rectA.h, rectB.y + rectB.h) - 
        std::max(rectA.y, rectB.y));

    // If both width and height are positive, calculate the overlap area
    if (overlapWidth > 0 && overlapHeight > 0) {
        return overlapWidth * overlapHeight;
    }

    // If there is no overlap, return 0
    return 0.0f;
}


void ResolveStaticCollision(Collider& colliderA, Physics& physicsA, const SDL_FPoint mtv)
{
    assert(colliderA.profile & Collider::Dynamic);
    if (colliderA.profile & Collider::NonSolid)
    {
        LOG_WARNING("ResolveStaticCollision called on a non-solid collider");
        return;
    }

    // Normalize MTV and calculate impulse
    SDL_FPoint normal = Normalize(mtv);
    float restitution = colliderA.material.restitution;

    float velocityAlongNormal = DotProduct(physicsA.velocity, normal);
    LOG_INFO_FMT("VelocityAlongNormal: {}", velocityAlongNormal);

    if (velocityAlongNormal < 0.0f)
    {
        // Apply impulse
        float impulse = -(1 + restitution) * velocityAlongNormal * physicsA.mass;
        SDL_FPoint impulseVec = { impulse * normal.x, impulse * normal.y };

        // Update velocity
        physicsA.velocity -= impulseVec;

        LOG_INFO_FMT("PhysicsA.velocity after impulse: {}", physicsA.velocity.y);
    }
}

//void ResolveStaticCollision(Collider& colliderA, Physics& physicsA, const SDL_FPoint mtv) 
//{
//    assert(colliderA.profile & Collider::Dynamic);
//    if (colliderA.profile & Collider::NonSolid)
//    {
//        LOG_WARNING("ResolveStaticCollision called on a non-solid collider");
//        return;
//    }
//
//    if (mtv.x != 0.0f || mtv.y != 0.0f)
//    {
//        int i = 0;
//    }
//
//    SDL_FPoint normal = Normalize(mtv);
//
//    float restitution = colliderA.material.restitution; 
//
//    float velocityAlongNormal = DotProduct(physicsA.velocity, normal);
//    LOG_INFO_FMT("VelocityAlongNormal: {}", velocityAlongNormal);
//    if (velocityAlongNormal < 0.0f) 
//    { 
//        float impulse = -(1 + restitution) * velocityAlongNormal;
//        impulse /= (1 / physicsA.mass); 
//
//        SDL_FPoint impulseVec = { impulse * normal.x, impulse * normal.y };
//
//        physicsA.velocity -= impulseVec / physicsA.mass;
//        LOG_INFO_FMT("PhysicsA.velocity.y: {}", physicsA.velocity.y);
//    }
//}

void ResolveDynamicCollision(Collider& colliderA, Physics& physicsA, Collider& colliderB, Physics& physicsB,
                             const SDL_FPoint mtv)
{
    assert(colliderA.profile & Collider::Dynamic);
    assert(colliderB.profile & Collider::Dynamic);

    if (colliderA.profile & Collider::NonSolid || colliderB.profile & Collider::NonSolid)
    {
        LOG_WARNING("ResolveDynamicCollision called on non-solid collider(s)");
        return;
    }

    SDL_FPoint relativeVelocity = physicsB.velocity - physicsA.velocity;

    SDL_FPoint normal = Normalize(mtv);

    float restitution = std::max(colliderA.material.restitution, 
                                 colliderB.material.restitution);

    float velocityAlongNormal = DotProduct(relativeVelocity, normal);
    if (velocityAlongNormal < 0.0f) // If objects are moving towards each other
    {
        float impulse = -(1 + restitution) * velocityAlongNormal;
        impulse /= (1 / physicsA.mass + 1 / physicsB.mass); // Apply based on mass

        SDL_FPoint impulseVec = { impulse * normal.x, impulse * normal.y };

        // Apply impulse to velocities
        physicsA.velocity -= impulseVec / physicsA.mass;
        physicsB.velocity += impulseVec / physicsB.mass;
    }

    SDL_FPoint tangent = relativeVelocity - normal * DotProduct(relativeVelocity, normal); // Normalized tangent direction
    float friction = std::sqrt(colliderA.material.friction * colliderB.material.friction);

    if (GetMagnitude(tangent) > 0.0f) 
    {
        float frictionImpulse = -DotProduct(relativeVelocity, tangent) * friction;
        physicsA.velocity += tangent * frictionImpulse / physicsA.mass;
        physicsB.velocity -= tangent * frictionImpulse / physicsB.mass;
    }
}


} // unnamed namespace

void DynamicCollisionsHelper::Rebuild(const std::vector<Entity>& entities)
{
    for (auto& entity : entities)
    {
        auto entityBounds = EntityColliderBounds::CreateFromEntity(entity, Collider::Profile::Dynamic);
        if (!entityBounds.has_value())
        {
            continue;
        }

        spatialGrid_.Insert(entityBounds->entityId, entityBounds->boundingBox);
    }
}

std::vector<EntityColliderBounds> DynamicCollisionsHelper::GetCollisionsWith(const Entity& entity, uint8_t flagsFilter)
{
    auto entityBounds = EntityColliderBounds::CreateFromEntity(entity, Collider::Profile::Dynamic);
    if (!entityBounds.has_value())
    {
        return {};
    }

    return spatialGrid_.GetIntersecting(entityBounds->entityId, entityBounds->boundingBox, flagsFilter);
}

StaticCollisionsHelper::StaticCollisionsHelper() : 
    quadTree_({ 0, 0, SDLite::kWindowWidth, SDLite::kWindowHeight})
{}

void StaticCollisionsHelper::Rebuild(Dimensions<int> sceneDims, std::vector<Entity>& entities)
{
    quadTree_ = QuadTree{ { 0.0f, 0.0f, static_cast<float>(sceneDims.w),
                                   static_cast<float>(sceneDims.h) } };

    for (auto& entity : entities)
    {
        auto entityBounds = EntityColliderBounds::CreateFromEntity(entity, Collider::Profile::Static);
        if (!entityBounds.has_value())
        {
            continue;
        }

        quadTree_.Insert(entityBounds->entityId, entityBounds->boundingBox);
    }
}

std::vector<EntityColliderBounds> StaticCollisionsHelper::GetCollisionsWith(const Entity& entity, uint8_t flagsFilter)
{
    auto entityBounds = EntityColliderBounds::CreateFromEntity(entity, Collider::Profile::Dynamic);
    if (!entityBounds.has_value())
    {
        return {};
    }

    return quadTree_.GetIntersecting(entityBounds->boundingBox, flagsFilter);
}

CollisionsStore CollisionSystem::GetCollisions(uint8_t flagsFilter)
{
    CollisionsStore store;

    auto dynamicEntities = ECS::GetAllEntitiesWith<Collider>([](const Collider& collider) {
        return collider.profile & Collider::Profile::Dynamic;
    });

    DynamicCollisionsHelper dynamicHelper{ dynamicEntities };

    for (auto& entity : dynamicEntities)
    {
        auto entityBounds = EntityColliderBounds::CreateFromEntity(entity, flagsFilter);
        if (!entityBounds.has_value())
        {
            continue;
        }

        auto intersectingStatic = staticCollisionsHelper_.GetCollisionsWith(entity, flagsFilter);
        auto intersectingDynamic = dynamicHelper.GetCollisionsWith(entity, flagsFilter);

        for (auto& statBounds : intersectingStatic)
        {
            store.staticCollisions.push_back({
                .a = *entityBounds,
                .b = statBounds
            });
        }
        for (auto& dynBounds : intersectingDynamic)
        {
            store.dynamicCollisions.push_back({
                .a = *entityBounds,
                .b = dynBounds
            });
        }
    }

    return store;
}

void CollisionSystem::HandleCollisions()
{
    auto store = GetCollisions();

    auto sendEvents = [](const std::vector<EntityCollision>& collisions)
    {
        for (const auto& col : collisions)
        {
            LOG_IF_ERROR(SendEventNotification(EntityCollision{ col }));
        }
    };

    sendEvents(store.staticCollisions);
    sendEvents(store.dynamicCollisions);

    for (int i = 0; i < kMaxRuns; i++)
    {
        store = GetCollisions(Collider::Solid);

        if (store.Empty())
        {
            break;
        }

        ResolveStaticCollisions(store.staticCollisions);
        ResolveDynamicCollisions(store.dynamicCollisions);
    }

    if (!store.Empty())
    {
        LOG_ERROR("Not all collisions could be resolved before hitting max runs");
    }
}

void CollisionSystem::RebuildQuadTree(Dimensions<int> sceneDims, std::vector<Entity>& staticEntities)
{
    staticCollisionsHelper_.Rebuild(sceneDims, staticEntities);
}

void CollisionSystem::ResolveStaticCollisions(const std::vector<EntityCollision>& collisions)
{
    for (const auto& [a, b] : collisions)
    {
        if (!a.boundingBox.Intersects(b.boundingBox))
        {
            continue;
        }
        LOG_DEBUG_FMT("Overlapping area between boxes: {}", CalculateOverlapArea(a.boundingBox, b.boundingBox));
        
        SDL_FPoint mtv = CalculateMTV(a.boundingBox, b.boundingBox);
        LOG_DEBUG_FMT("mtv magnitude: {}", GetMagnitude(mtv));

        auto dynEntity = ECS::GetEntityByID(a.entityId);
        assert(dynEntity.IsValid());

        assert(dynEntity.HasComponent<Collider>());
        auto& dynCollider = dynEntity.GetComponent<Collider>();

        assert(dynEntity.HasComponent<Physics>());
        auto& dynPhysics = dynEntity.GetComponent<Physics>();

        ResolveStaticCollision(dynCollider, dynPhysics, mtv);

        //LOG_DEBUG_FMT("COLLIDER POSITION Y BEFORE -= MTV: {}", dynCollider.position.y);
        dynCollider.position -= mtv;
        //LOG_DEBUG_FMT("COLLIDER POSITION Y AFTER -= MTV: {}", dynCollider.position.y);   

        auto newBbox = AABB{
            dynCollider.position.x - (dynCollider.dimensions.w / 2.0f),
            dynCollider.position.y - (dynCollider.dimensions.h / 2.0f),
            dynCollider.dimensions.w, dynCollider.dimensions.h
        };
        //assert(!newBbox.Intersects(b.boundingBox));
    }
}

void CollisionSystem::ResolveDynamicCollisions(const std::vector<EntityCollision>& collisions)
{
    for (const auto& [a, b] : collisions)
    {
        if (!a.boundingBox.Intersects(b.boundingBox))
        {
            continue;
        }

        SDL_FPoint mtv = CalculateMTV(a.boundingBox, b.boundingBox);

        auto entA = ECS::GetEntityByID(a.entityId);
        auto entB = ECS::GetEntityByID(b.entityId);
        assert(entA.IsValid());
        assert(entB.IsValid());

        assert(entA.HasComponent<Collider>());
        assert(entB.HasComponent<Collider>());

        auto& colliderA = entA.GetComponent<Collider>();
        auto& colliderB = entB.GetComponent<Collider>();

        assert(entA.HasComponent<Physics>());
        assert(entB.HasComponent<Physics>());

        auto& physicsA = entA.GetComponent<Physics>();
        auto& physicsB = entB.GetComponent<Physics>();

        ResolveDynamicCollision(colliderA, physicsA, colliderB, physicsB, mtv);

        colliderA.position -= mtv;
        colliderB.position += mtv;
    }
}

SDL_FPoint CollisionSystem::CalculateMTV(const AABB& a, const AABB& b)
{
    float dx = (a.GetCenter().x - b.GetCenter().x);
    float px = (a.GetSizeHalf().w + b.GetSizeHalf().w) - std::abs(dx);

    float dy = (a.GetCenter().y - b.GetCenter().y);
    float py = (a.GetSizeHalf().h + b.GetSizeHalf().h) - std::abs(dy);

    if (px < py)
    {
        return { (dx < 0.0f ? -px : px), 0.0f };
    }
    else
    {
        return { 0.0f, (dy < 0.0f ? -py : py) };
    }
}
