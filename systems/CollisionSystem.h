#pragma once
#include "util/SpatialHashGrid.h"
#include "../core/entity/EntityColliderBounds.h"
#include "../events/custom/CustomEventDataRegistry.h"

class DynamicCollisionsHelper
{
public:
    DynamicCollisionsHelper() = default;
    explicit DynamicCollisionsHelper(const std::vector<Entity>& entities) { Rebuild(entities); }

    void Rebuild(const std::vector<Entity>& entities);
    std::vector<EntityColliderBounds> GetCollisionsWith(const Entity& entity, uint8_t flagsFilter = 0xFF);

private:
    SpatialHashGrid spatialGrid_;
};

class StaticCollisionsHelper
{
public:
    //StaticCollisionsHelper() : quadTree_({ 0.0f, 0.0f, 0.0f, 0.0f }) {}
    StaticCollisionsHelper();

    void Rebuild(Dimensions<int> sceneDims, std::vector<Entity>& entities);

    std::vector<EntityColliderBounds> GetCollisionsWith(const Entity& entity, uint8_t flagsFilter = 0xFF);

private:
    QuadTree quadTree_;
}; 

struct CollisionsStore
{
    std::vector<EntityCollision> staticCollisions;
    std::vector<EntityCollision> dynamicCollisions;
    bool Empty() const { return staticCollisions.empty() && dynamicCollisions.empty(); }
};

class CollisionSystem
{
public:
    static constexpr int kMaxRuns = 8;

    CollisionsStore GetCollisions(uint8_t flagsFilter = 0x00);

    void HandleCollisions();

    void RebuildQuadTree(Dimensions<int> sceneDims, std::vector<Entity>& staticEntities);

private:
    void ResolveStaticCollisions(const std::vector<EntityCollision>& collisions);
    void ResolveDynamicCollisions(const std::vector<EntityCollision>& collisions);

    static SDL_FPoint CalculateMTV(const AABB& a, const AABB& b);

    StaticCollisionsHelper staticCollisionsHelper_;
};