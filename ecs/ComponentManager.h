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

    static constexpr uint16_t invalidIndex = std::numeric_limits<uint16_t>::max();

    ComponentArray()
    {
        isEntityHoldsComponentIndex_.fill(invalidIndex);
    }

    T& AddComponent(Entity_t entity, T component = {})
    {
        assert(entity < kMaxEntities);

        uint16_t cmpIndex = isEntityHoldsComponentIndex_[entity];

        if (cmpIndex != invalidIndex)
        {
            return components_[cmpIndex];
        }

        cmpIndex = static_cast<uint16_t>(components_.size());

        isComponentIndexHoldsEntity_.emplace_back(entity);
        isEntityHoldsComponentIndex_[entity] = cmpIndex;

        return components_.emplace_back(std::move(component));
    }

    void RemoveComponent(Entity_t entity)
    {
        assert(entity < kMaxEntities);

        const uint16_t cmpIndex = isEntityHoldsComponentIndex_[entity];

        if (cmpIndex == invalidIndex)
        {
            return;
        }

        const uint16_t lastIdx = static_cast<uint16_t>(components_.size() - 1);

        if (cmpIndex != lastIdx)
        {
            const Entity_t lastEntity = isComponentIndexHoldsEntity_[lastIdx];

            std::swap(components_[cmpIndex], components_[lastIdx]);
            std::swap(isComponentIndexHoldsEntity_[cmpIndex], isComponentIndexHoldsEntity_[lastIdx]);

            isEntityHoldsComponentIndex_[lastEntity] = cmpIndex;
        }

        components_.pop_back();
        isComponentIndexHoldsEntity_.pop_back();

        isEntityHoldsComponentIndex_[entity] = invalidIndex;
    }

    T& GetComponent(Entity_t entity)
    {
        assert(entity < kMaxEntities);

        const uint16_t cmpIndex = isEntityHoldsComponentIndex_[entity];

        assert(cmpIndex != invalidIndex);

        return components_[cmpIndex];
    }

    const T& GetComponent(Entity_t entity) const
    {
        assert(entity < kMaxEntities);

        const uint16_t cmpIndex = isEntityHoldsComponentIndex_[entity];

        assert(cmpIndex != invalidIndex);

        return components_[cmpIndex];
    }

private:
    std::vector<T> components_;
    std::vector<Entity_t> isComponentIndexHoldsEntity_;
    std::array<uint16_t, kMaxEntities> isEntityHoldsComponentIndex_;
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
    T& AddComponent(Entity_t entity, T component)
    {
        assert(entity < kMaxEntities);

        isEntityHoldsSignature_[entity] |= T::componentBit;

        auto& entry = GetEntry<T>();

        return entry.AddComponent(entity, std::move(component));
    }

    template <typename T>
    T& AddComponent(Entity_t entity)
    {
        assert(entity < kMaxEntities);

        isEntityHoldsSignature_[entity] |= T::componentBit;

        auto& entry = GetEntry<T>();

        return entry.AddComponent(entity, {});
    }

    template <typename T>
    void RemoveComponent(Entity_t entity)
    {
        isEntityHoldsSignature_[entity] &= ~(T::componentBit);

        auto& entry = GetEntry<T>();

        return entry.RemoveComponent(entity);
    }

    template <typename T>
    T& GetComponent(Entity_t entity)
    {
        assert(entity < kMaxEntities);
        assert(isEntityHoldsSignature_[entity] & T::componentBit);

        auto& entry = GetEntry<T>();

        return entry.GetComponent(entity);
    }

    template <typename T>
    const T& GetComponent(Entity_t entity) const
    {
        assert(entity < kMaxEntities);
        assert(isEntityHoldsSignature_[entity] & T::componentBit);

        const auto& entry = GetEntry<T>();

        return entry.GetComponent(entity);
    }

    template <typename T>
    bool HasComponent(Entity_t entity) const
    {
        assert(entity < kMaxEntities);

        return isEntityHoldsSignature_[entity] & T::componentBit;
    }

    void EntityCreated(Entity_t entity)
    {
        assert(entity < kMaxEntities);

        isEntityHoldsSignature_[entity] = ActiveState::componentBit;
    }

    void EntityDestroyed(Entity_t entity)
    {
        assert(entity < kMaxEntities);

        ForEachInTuple(componentArrays_, [entity](auto& componentArray) {
            componentArray.RemoveComponent(entity);
        });

        isEntityHoldsSignature_[entity] = 0;
    }

    ComponentSignature GetSignature(Entity_t entity) const
    {
        assert(entity < kMaxEntities);

        return isEntityHoldsSignature_[entity];
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
    std::array<ComponentSignature, kMaxEntities> isEntityHoldsSignature_{};
};