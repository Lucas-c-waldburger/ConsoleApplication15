#pragma once
#include "EntityT.h"
#include <span>
#include <array>

class EntityManager
{
public:
    using FlatIdx = EntityIndex_t;

    EntityManager();

    Entity_t CreateEntity();

    void DestroyEntity(Entity_t entityToRemove);

    std::span<const Entity_t> GetActiveEntities() const;

    bool IsEntityActive(Entity_t entity) const;

private:
    std::array<Entity_t, kMaxEntities> indexWithFlatIdxToGetEntity_t_;
    std::array<FlatIdx, kMaxEntities> indexWithEntityIdxToGetFlatIdx_;
    FlatIdx nextFreeFlatIndex_ = 0;
};