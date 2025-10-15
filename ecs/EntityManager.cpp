#include "EntityManager.h"
#include <cassert>

EntityManager::EntityManager()
{
    for (Entity_t entity = 0; entity < kMaxEntities; entity++)
    {
        const EntityIndex_t entityIndex = GetEntity_tIndex(entity);
        const FlatIdx flatIndex = entityIndex;

        indexWithFlatIdxToGetEntity_t_[flatIndex] = entity;
        indexWithEntityIdxToGetFlatIdx_[entityIndex] = flatIndex;
    }
}

Entity_t EntityManager::CreateEntity()
{
    assert(nextFreeFlatIndex_ < kMaxEntities);

    const Entity_t entityOfLastGen =
        indexWithFlatIdxToGetEntity_t_[nextFreeFlatIndex_];

    const EntityIndex_t entityIndex = GetEntity_tIndex(entityOfLastGen);

    indexWithEntityIdxToGetFlatIdx_[entityIndex] = nextFreeFlatIndex_;

    const Entity_t entityOfNextGen = IncrementEntity_tGeneration(entityOfLastGen);

    indexWithFlatIdxToGetEntity_t_[nextFreeFlatIndex_] = entityOfNextGen;

    ++nextFreeFlatIndex_;

    return entityOfNextGen;
}

void EntityManager::DestroyEntity(Entity_t entityToRemove)
{
    assert(nextFreeFlatIndex_ > 0);

    const EntityIndex_t entityToRemoveEntityIndex = GetEntity_tIndex(entityToRemove);
    assert(entityToRemoveEntityIndex <= kMaxEntityIndex);

    const FlatIdx entityToRemoveFlatIndex =
        indexWithEntityIdxToGetFlatIdx_[entityToRemoveEntityIndex];

    if (entityToRemoveFlatIndex >= nextFreeFlatIndex_) // this is already 'erased'
    {
        return;
    }

    if (nextFreeFlatIndex_ > 1) // this isnt the only entity active
    {
        const FlatIdx lastActiveFlatIndex = nextFreeFlatIndex_ - 1;
        const Entity_t lastActiveEntity =
            indexWithFlatIdxToGetEntity_t_[lastActiveFlatIndex];
        const EntityIndex_t lastActiveEntityIndex = GetEntity_tIndex(lastActiveEntity);

        indexWithEntityIdxToGetFlatIdx_[entityToRemoveEntityIndex] =
            lastActiveFlatIndex;
        indexWithFlatIdxToGetEntity_t_[lastActiveFlatIndex] =
            entityToRemove;

        indexWithEntityIdxToGetFlatIdx_[lastActiveEntityIndex] =
            entityToRemoveFlatIndex;
        indexWithFlatIdxToGetEntity_t_[entityToRemoveFlatIndex] =
            lastActiveEntity;
    }

    --nextFreeFlatIndex_;
}

std::span<const Entity_t> EntityManager::GetActiveEntities() const
{
    return std::span<const Entity_t>{
        indexWithFlatIdxToGetEntity_t_}.subspan(0, nextFreeFlatIndex_);
}

bool EntityManager::IsEntityActive(Entity_t entity) const
{
    const EntityIndex_t entityIndex = GetEntity_tIndex(entity);

    assert(entityIndex <= kMaxEntityIndex);

    const FlatIdx flatIndex = indexWithEntityIdxToGetFlatIdx_[entityIndex];

    return flatIndex < nextFreeFlatIndex_ &&
           indexWithFlatIdxToGetEntity_t_[flatIndex] == entity;
}