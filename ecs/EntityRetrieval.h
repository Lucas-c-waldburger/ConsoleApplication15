#pragma once
//#include "EntityManager.h"
//#include "ComponentManager.h"

//struct SignatureMask
//{
//    uint64_t include = 0;
//    uint64_t exclude = 0;
//    uint64_t hasAnyOf = 0;
//
//    template <typename...Ts>
//    static SignatureMask Create()
//    {
//        SignatureMask sigMask{};
//
//        (ProcessComponentBit<Ts>(sigMask), ...);
//
//        return sigMask;
//    }
//
//private:
//    template <typename T>
//    static void ProcessComponentBit(SignatureMask& sigMask)
//    {
//        if constexpr (is_exclude<T>::value)
//        {
//            sigMask.exclude |= GetWrappedBit(T{});
//        }
//        else if constexpr (is_any_of_wrapper<T>::value)
//        {
//            sigMask.hasAnyOf |= ExpandWrappedBits(T{});
//        }
//        //else if constexpr (is_upcast_to_wrapper<T>::value)
//        //{
//        //    sigMask.include |= ExpandWrappedBits(T{});
//        //}
//        else
//        {
//            sigMask.include |= T::componentBit;
//        }
//    }
//
//
//
//    //template <template <typename...> class Wrapper, ComponentType T, ComponentType...Ts>
//    //static constexpr uint64_t ExpandWrappedBits(Wrapper<T, Ts...> wrapper)
//    //{
//    //    return (Ts::componentBit | ...);
//    //}
//
//    template <template <typename...> class Wrapper, ComponentType...Ts>
//    static constexpr uint64_t ExpandWrappedBits(Wrapper<Ts...> wrapper)
//    {
//        return (Ts::componentBit | ...);
//    }
//
//    template <template <typename> class Wrapper, ComponentType T>
//    static constexpr uint64_t GetWrappedBit(Wrapper<T> wrapper)
//    {
//        return T::WrappedType::componentBit;
//    }
//};
//
//
//class EntityRetrieval
//{
//public:
//    template <typename...Ts>
//    static std::vector<Entity_t> GetAllEntitiesWith(EntityManager& entityManager, 
//                                                    ComponentManager& componentManager)
//    {
//        std::vector<Entity_t> result;
//        auto sigMask = SignatureMask::Create<Ts...>();
//
//        auto activeEntities = entityManager.GetActiveEntities();
//        for (const auto& entity : activeEntities)
//        {
//            const uint64_t entitySig = componentManager.GetSignature(entity);
//
//            if ((entitySig & sigMask.hasAnyOf) &
//              ~((entitySig & sigMask.include) ^ sigMask.include) &
//               ~(entitySig & sigMask.exclude))
//            {
//                result.push_back(entity);
//            }
//        }
//
//        return result; 
//    } 
//};

