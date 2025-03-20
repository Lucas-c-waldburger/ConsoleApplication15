#pragma once
#include "Core.h"
#include <cassert>
#include <span>
#include <array>

class EntityManager
{
public:
    EntityManager()
    {
        for (Entity_t i = 0; i < kMaxEntities; i++)
        {
            isIndexHoldsEntity_[i] = i;
            isEntityHoldsIndex_[i] = i;
        }
    }

    Entity_t CreateEntity()
    {
        assert(nextFreeIndex_ < kMaxEntities);

        const Entity_t nextEntity = isIndexHoldsEntity_[nextFreeIndex_];

        isEntityHoldsIndex_[nextEntity] = nextFreeIndex_;

        ++nextFreeIndex_;

        return nextEntity;
    }

    void DestroyEntity(Entity_t entityToRemove)
    {
        assert(entityToRemove < kMaxEntities);
        assert(nextFreeIndex_ > 0);

        const uint16_t entityToRemoveIndex = isEntityHoldsIndex_[entityToRemove];
        if (entityToRemoveIndex >= nextFreeIndex_) // this is already 'erased'
        {
            return;
        }

        if (nextFreeIndex_ > 1) // this isnt the only entity active entity
        {
            const uint16_t lastActiveIndex = nextFreeIndex_ - 1;
            const Entity_t lastActiveEntity = isIndexHoldsEntity_[lastActiveIndex];

            isEntityHoldsIndex_[entityToRemove] = lastActiveIndex;
            isIndexHoldsEntity_[lastActiveIndex] = entityToRemove;

            isEntityHoldsIndex_[lastActiveEntity] = entityToRemoveIndex;
            isIndexHoldsEntity_[entityToRemoveIndex] = lastActiveEntity;
        }

        --nextFreeIndex_;
    }

    std::span<const Entity_t> GetActiveEntities() const
    {
        return std::span<const Entity_t>{isIndexHoldsEntity_}.subspan(0, nextFreeIndex_);
    }

private:
    std::array<Entity_t, kMaxEntities> isIndexHoldsEntity_;
    std::array<uint16_t, kMaxEntities> isEntityHoldsIndex_;
    uint16_t nextFreeIndex_ = 0;
};