#pragma once
#include <array>
#include <vector>
#include <cassert>
#include <algorithm>
#include "EntityT.h"
#include "../components/ComponentIncludes.h"

template <typename T>
class ComponentArray
{
public:
    using ValueType = T;
    using ComponentIndex_t = size_t;

    static constexpr ComponentIndex_t kInvalidComponentIndex = 
        std::numeric_limits<ComponentIndex_t>::max();

    ComponentArray()
    {
        indexWithEntityIdxToGetComponentIdx_.fill(kInvalidComponentIndex);
    }

    T& AddComponent(Entity_t entity, T&& component)
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntityIndex);

        ComponentIndex_t cmpIndex = indexWithEntityIdxToGetComponentIdx_[entityIndex];

        if (cmpIndex != kInvalidComponentIndex)
        {
            // entity already has this component, just return it
            return components_[cmpIndex];
        }

        cmpIndex = components_.size();

        indexWithComponentIdxToGetEntity_.emplace_back(entity);
        indexWithEntityIdxToGetComponentIdx_[entityIndex] = cmpIndex;

        return components_.emplace_back(std::forward<T>(component));
    }

    T& AddComponent(Entity_t entity)
    {
        return AddComponent(entity, T{});
    }

    void RemoveComponent(Entity_t entity)
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntityIndex);

        const ComponentIndex_t cmpIndex = 
            indexWithEntityIdxToGetComponentIdx_[entityIndex];

        if (cmpIndex == kInvalidComponentIndex)
        {
            return;
        }

        const ComponentIndex_t lastIdx = components_.size() - 1;

        if (cmpIndex != lastIdx)
        {
            const Entity_t lastEntity = indexWithComponentIdxToGetEntity_[lastIdx];

            std::swap(components_[cmpIndex], components_[lastIdx]);
            std::swap(indexWithComponentIdxToGetEntity_[cmpIndex], 
                      indexWithComponentIdxToGetEntity_[lastIdx]);

            indexWithEntityIdxToGetComponentIdx_[GetEntityIndex(lastEntity)] = cmpIndex;
        }

        components_.pop_back();
        indexWithComponentIdxToGetEntity_.pop_back();

        indexWithEntityIdxToGetComponentIdx_[entityIndex] = kInvalidComponentIndex;
    }

    T& GetComponent(Entity_t entity)
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntityIndex);

        const ComponentIndex_t cmpIndex = 
            indexWithEntityIdxToGetComponentIdx_[entityIndex];

        assert(cmpIndex != kInvalidComponentIndex);

        return components_[cmpIndex];
    }

    const T& GetComponent(Entity_t entity) const
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntityIndex);

        const ComponentIndex_t cmpIndex = 
            indexWithEntityIdxToGetComponentIdx_[entityIndex];

        assert(cmpIndex != kInvalidComponentIndex);

        return components_[cmpIndex];
    }

private:
    std::vector<T> components_;
    std::vector<Entity_t> indexWithComponentIdxToGetEntity_;
    std::array<size_t, kMaxEntityIndex> indexWithEntityIdxToGetComponentIdx_;
};

namespace detail {
    template <typename TList>
    struct component_array_tuple;

    template <typename...Ts>
    struct component_array_tuple<TypeList<Ts...>>
    {
        using type = std::tuple<ComponentArray<Ts>...>;
    };
} // detail

using ComponentArrayTuple = detail::component_array_tuple<ComponentTypeList>::type;


class ComponentManager
{
public:
    template <typename T>
    T& AddComponent(Entity_t entity, T&& component)
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntities);

        indexWithEntityIdxToGetComponentSignature_[entityIndex] |= T::componentBit;

        auto& entry = GetEntry<T>();

        return entry.AddComponent(entity, std::forward<T>(component));
    }

    template <typename T>
    T& AddComponent(Entity_t entity)
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntities);

        indexWithEntityIdxToGetComponentSignature_[entityIndex] |= T::componentBit;

        auto& entry = GetEntry<T>();

        return entry.AddComponent(entity);
    }

    template <typename T>
    void RemoveComponent(Entity_t entity)
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntities);

        indexWithEntityIdxToGetComponentSignature_[entityIndex] &= ~(T::componentBit);

        auto& entry = GetEntry<T>();

        return entry.RemoveComponent(entity);
    }

    template <typename T>
    T& GetComponent(Entity_t entity)
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntities);
        assert(indexWithEntityIdxToGetComponentSignature_[entityIndex] & T::componentBit);

        auto& entry = GetEntry<T>();

        return entry.GetComponent(entity);
    }

    template <typename T>
    const T& GetComponent(Entity_t entity) const
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntities);
        assert(indexWithEntityIdxToGetComponentSignature_[entityIndex] & T::componentBit);

        const auto& entry = GetEntry<T>();

        return entry.GetComponent(entity);
    }

    template <typename T>
    bool HasComponent(Entity_t entity) const
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntities);

        return indexWithEntityIdxToGetComponentSignature_[entityIndex] & T::componentBit;
    }

    void EntityCreated(Entity_t entity)
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntities);

        indexWithEntityIdxToGetComponentSignature_[entityIndex] = 
            ActiveState::componentBit;
    }

    void EntityDestroyed(Entity_t entity)
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntities);

        ForEachInTuple(componentArrays_, [entity](auto& componentArray) {
            componentArray.RemoveComponent(entity);
        });

        indexWithEntityIdxToGetComponentSignature_[entityIndex] = 0;
    }

    ComponentSignature GetSignature(Entity_t entity) const
    {
        const auto entityIndex = GetEntityIndex(entity);

        assert(entityIndex < kMaxEntities);

        return indexWithEntityIdxToGetComponentSignature_[entityIndex];
    }

protected:
    template <typename T>
    ComponentArray<T>& GetEntry()
    {
        return std::get<ComponentArray<T>>(componentArrays_);
    }
    template <typename T>
    const ComponentArray<T>& GetEntry() const
    {
        return std::get<ComponentArray<T>>(componentArrays_);
    }

    ComponentArrayTuple componentArrays_;
    std::array<ComponentSignature, kMaxEntityIndex> indexWithEntityIdxToGetComponentSignature_{};
};