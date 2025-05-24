//#pragma once
//#include "Entity.h"
//#include "../core/TypeUtils.h"
//
//class EntityRetrievalHelper
//{
//public:
//    template <ComponentType...Ts, typename Filter>
//    static std::vector<Entity> GetAllEntitiesWithFiltered(ECSImpl& ecsImpl, EntityManager& entManager, 
//                                                          impl::ComponentManager&, Filter&& filter);
//
//    template <typename...Ts>
//    std::vector<Entity> GetAllEntitiesWith(ECSImpl& ecsImpl, EntityManager& entManager,
//                                           impl::ComponentManager&);
//
//private:
//	EntityRetrievalHelper() = default;
//};
//
//template<ComponentType ...Ts, typename Filter>
//inline std::vector<Entity> EntityRetrievalHelper::GetAllEntitiesWithFiltered(ECSImpl& ecsImpl, 
//                                                                             EntityManager& entManager, 
//                                                                             impl::ComponentManager&, Filter&& filter)
//{
//    std::vector<Entity> result;
//    const uint64_t mask = (Ts::componentBit | ...);
//
//    auto activeEntities = entManager.GetActiveEntities();
//
//    for (const auto& ent : activeEntities)
//    {
//        const uint64_t entitySig = cmpManager.GetSignature(ent);
//
//        if ((entitySig & mask) != mask)
//        {
//            continue;
//        }
//
//        if constexpr (HasBooleanNotOperator<Filter>)
//        {
//            if (!filter)
//            {
//                continue;
//            }
//        }
//
//        if (!std::invoke(filter, cmpManager.GetComponent<Ts>(ent)...))
//        {
//            continue;
//        }
//
//        result.emplace_back(ent, ecsImpl);
//    }
//
//    return result;
//}
//
//template<typename ...Ts>
//inline std::vector<Entity> EntityRetrievalHelper::GetAllEntitiesWith(ECSImpl& ecsImpl, 
//                                                                     EntityManager& entManager, 
//                                                                     impl::ComponentManager&)
//{
//    std::vector<Entity> result;
//    uint64_t excludeMask = 0;
//
//    auto makeMasks = [&excludeMask]<typename T>() -> uint64_t {
//        if constexpr (is_exclude<T>::value)
//        {
//            excludeMask |= T::WrappedType::componentBit;
//            return 0;
//        }
//        else
//        {
//            return T::componentBit;
//        }
//    };
//
//    const uint64_t includeMask = (makeMasks.template operator()<Ts>() | ...);
//
//    auto activeEntities = entManager.GetActiveEntities();
//
//    for (const auto& ent : activeEntities)
//    {
//        const uint64_t entitySig = cmpManager.GetSignature(ent);
//
//        if (((entitySig & includeMask) != includeMask) || (entitySig & excludeMask))
//        {
//            continue;
//        }
//
//        result.emplace_back(ent, ecsImpl);
//    }
//
//    return result;
//}
