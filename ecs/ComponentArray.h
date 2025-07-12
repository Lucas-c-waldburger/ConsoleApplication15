//#pragma once
//#include <vector>
//#include <algorithm>
//#include <cassert>
//#include <array>
//#include "EntityT.h"
//
//
//template <typename T>
//class ComponentArray
//{
//public:
//    using ValueType = T;
//
//    static constexpr uint16_t invalidIndex = std::numeric_limits<uint16_t>::max();
//
//    ComponentArray()
//    {
//        isEntityHoldsComponentIndex_.fill(invalidIndex);
//    }
//
//    T& AddComponent(Entity_t entity, T component = {})
//    {
//        assert(entity < kMaxEntities);
//
//        uint16_t cmpIndex = isEntityHoldsComponentIndex_[entity];
//
//        if (cmpIndex != invalidIndex)
//        {
//            return components_[cmpIndex];
//        }
//
//        cmpIndex = static_cast<uint16_t>(components_.size());
//
//        isComponentIndexHoldsEntity_.emplace_back(entity);
//        isEntityHoldsComponentIndex_[entity] = cmpIndex;
//
//        return components_.emplace_back(std::move(component));
//    }
//
//    void RemoveComponent(Entity_t entity)
//    {
//        assert(entity < kMaxEntities);
//
//        const uint16_t cmpIndex = isEntityHoldsComponentIndex_[entity];
//
//        if (cmpIndex == invalidIndex)
//        {
//            return;
//        }
//
//        const uint16_t lastIdx = static_cast<uint16_t>(components_.size() - 1);
//
//        if (cmpIndex != lastIdx)
//        {
//            const Entity_t lastEntity = isComponentIndexHoldsEntity_[lastIdx];
//
//            std::swap(components_[cmpIndex], components_[lastIdx]);
//            std::swap(isComponentIndexHoldsEntity_[cmpIndex], isComponentIndexHoldsEntity_[lastIdx]);
//
//            isEntityHoldsComponentIndex_[lastEntity] = cmpIndex;
//        }
//
//        components_.pop_back();
//        isComponentIndexHoldsEntity_.pop_back();
//
//        isEntityHoldsComponentIndex_[entity] = invalidIndex;
//    }
//
//    T& GetComponent(Entity_t entity)
//    {
//        assert(entity < kMaxEntities);
//
//        const uint16_t cmpIndex = isEntityHoldsComponentIndex_[entity];
//
//        assert(cmpIndex != invalidIndex);
//
//        return components_[cmpIndex];
//    }
//
//    const T& GetComponent(Entity_t entity) const
//    {
//        assert(entity < kMaxEntities);
//
//        const uint16_t cmpIndex = isEntityHoldsComponentIndex_[entity];
//
//        assert(cmpIndex != invalidIndex);
//
//        return components_[cmpIndex];
//    }
//
//private:
//    std::vector<T> components_;
//    std::vector<Entity_t> isComponentIndexHoldsEntity_;
//    std::array<uint16_t, kMaxEntities> isEntityHoldsComponentIndex_;
//};