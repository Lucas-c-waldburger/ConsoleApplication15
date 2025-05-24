//#pragma once
//#include "EntityManager.h"
//#include "EntityRelationsHelper.h"
//#include "../core/TypeUtils.h"
//#include <cassert>
//#include <functional>
//
//// TODO: Do we need an ActiveComponent if we can just get that info directly from EntityManager?
//
//class ECSImpl
//{
//private:
//    /*class Accessor
//    {
//    public:
//        explicit Accessor(ECSImpl& ecs) : ecs_(&ecs) {}
//
//    protected:
//        bool IsValid() const { return ecs_ != nullptr; }
//
//        EntityManager& GetEntityManager();
//        const EntityManager& GetEntityManager() const;
//        impl::ComponentManager& GetComponentManager();
//        const impl::ComponentManager& GetComponentManager() const;
//
//    private:
//        ECSImpl* ecs_;
//    };*/
//
//public:
//    //friend class Accessor;
//    friend class ECS;
//    friend class Entity;
//    friend class EntityRelations;
//
//    ~ECSImpl() = default;
//
//    ECSImpl(const ECSImpl&) = delete;
//    ECSImpl(ECSImpl&&) = delete;
//    ECSImpl& operator=(const ECSImpl&) = delete;
//    ECSImpl& operator=(ECSImpl&&) = delete;
//
//private:
//    Entity_t CreateEntity_t()
//    {
//        Entity_t entity = entityManager_.CreateEntity();
//        componentManager_.EntityCreated(entity);
//
//        return entity;
//    }
//
//    void DestroyEntity(Entity_t entity)
//    {
//        entityManager_.DestroyEntity(entity);
//        EntityRelationsHelper::DestroyRelationshipsWithEntity(componentManager_, entity);
//        componentManager_.EntityDestroyed(entity);
//    }
//
//    template <ComponentType T> requires (!RelationalComponentType<T>)
//        T& AddComponent(Entity_t entity, T cmp = {})
//    {
//        return componentManager_.AddComponent<T>(entity, std::move(cmp));
//    }
//
//    template <RelationalComponentType T>
//    T& AddComponent(Entity_t entity, T cmp)
//    {
//        return componentManager_.AddComponent<T>(entity, std::move(cmp));
//    }
//
//    template <ComponentType T> requires (!RelationalComponentType<T>)
//        void RemoveComponent(Entity_t entity)
//    {
//        return componentManager_.RemoveComponent<T>(entity);
//    }
//
//    template <ComponentType T> requires (!RelationalComponentType<T>)
//        T& GetComponent(Entity_t entity)
//    {
//        return componentManager_.GetComponent<T>(entity);
//    }
//
//    template <ComponentType T>
//    const T& GetComponent(Entity_t entity) const // all relationship stuff has to be done through relations API
//    {
//        return componentManager_.GetComponent<T>(entity);
//    }
//
//    template <ComponentType...Ts> requires (!RelationalComponentType<Ts> && ...)
//        std::tuple<Ts&...> GetComponents(Entity_t entity)
//    {
//        return std::tie(componentManager_.GetComponent<Ts>(entity)...);
//    }
//
//    template <ComponentType T>
//    bool HasComponent(Entity_t entity) const
//    {
//        return componentManager_.GetSignature(entity) & T::componentBit;
//    }
//
//    bool IsEntityActive(Entity_t entity) const
//    {
//        assert(entity < kMaxEntities);
//
//        bool activeAccordingToComponentManager =
//            componentManager_.GetSignature(entity) & ActiveState::componentBit;
//        bool activeAccordingToEntityManager = entityManager_.IsEntityActive(entity);
//
//        assert(activeAccordingToComponentManager == activeAccordingToEntityManager);
//
//        return activeAccordingToComponentManager;
//    }
//
//    bool SetParent(Entity_t entity, Entity_t requestedParent)
//    {
//        if (!IsEntityActive(entity) || !IsEntityActive(requestedParent)) { return false; }
//
//        bool result = EntityRelationsHelper::SetParent(componentManager_, entity, requestedParent);
//
//        assert(!(componentManager_.HasComponent<Children>(entity) &&
//                 componentManager_.HasComponent<Parent>(entity)));
//        assert(!(componentManager_.HasComponent<Children>(requestedParent) &&
//                 componentManager_.HasComponent<Parent>(requestedParent)));
//
//        return result;
//    }
//
//    bool RemoveParent(Entity_t entity)
//    {
//        if (!IsEntityActive(entity)) { return false; }
//
//        return EntityRelationsHelper::SetParent(componentManager_, entity, kInvalidEntity);
//    }
//
//    bool AddChild(Entity_t entity, Entity_t requestedChild)
//    {
//        if (!IsEntityActive(entity) || !IsEntityActive(requestedChild)) { return false; }
//
//        bool result = EntityRelationsHelper::AddChild(componentManager_, entity, requestedChild);
//
//        assert(!(componentManager_.HasComponent<Children>(entity) &&
//                 componentManager_.HasComponent<Parent>(entity)));
//        assert(!(componentManager_.HasComponent<Children>(requestedChild) &&
//                 componentManager_.HasComponent<Parent>(requestedChild)));
//
//        return result;
//    }
//
//    bool RemoveChild(Entity_t entity, Entity_t requestedChild)
//    {
//        if (!IsEntityActive(entity) || !IsEntityActive(requestedChild)) { return false; }
//
//        return EntityRelationsHelper::RemoveChild(componentManager_, entity, requestedChild);
//    }
//
//    static ECSImpl& Get()
//    {
//        static std::unique_ptr<ECSImpl> ecs;
//        if (!ecs)
//        {
//            ecs = std::unique_ptr<ECSImpl>(new ECSImpl());
//        }
//
//        return *ecs;
//    }
//
//    ECSImpl() = default;
//
//    EntityManager entityManager_;
//    impl::ComponentManager componentManager_;
//};
//
//
//
