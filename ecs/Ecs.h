#pragma once
#include "EntityManager.h"
#include "ComponentManager.h"
#include "EntityRelationsHelper.h"
#include "EntityAccess.h"
#include "../components/ComponentConcepts.h"
#include "../core/Logger.h"
#include <cassert>
#include <functional>

class ECS;
class EntityRelations;

// ENTITY //
class Entity
{
public:
    // components that can't be mutated through Entity API (must use EntityPassKey)
    template <SomeComponent T>
    static constexpr bool public_mutable_component_v = (
        !(RelationalComponentType<T>   ||
          std::same_as<T, EntityFlags> ||
          std::same_as<T, ActiveState> ||
          std::same_as<T, ActiveAudio>)
    );

    Entity() : id_(kInvalidEntity), ecs_(nullptr) {}
    Entity(Entity_t id, ECS& ecs) : id_(id), ecs_(&ecs) {}

    template <SomeComponent T> requires Entity::public_mutable_component_v<T>
    T& AddComponent(T&& cmp);
    template <SomeComponent T> requires Entity::public_mutable_component_v<T>
    T& AddComponent();
    template <SomeComponent T> 
    T& AddComponent(T&& cmp, EntityPassKey);
    template <SomeComponent T>
    T& AddComponent(EntityPassKey);

    template <SomeComponent T> requires Entity::public_mutable_component_v<T>
    void RemoveComponent();
    template <SomeComponent T>
    void RemoveComponent(EntityPassKey);

    void ClearComponents();

    template <SomeComponent T> requires Entity::public_mutable_component_v<T>
    T& GetComponent();
    template <SomeComponent T>
    T& GetComponent(EntityPassKey);
    template <SomeComponent T> 
    const T& GetComponent() const;

    template <SomeComponent T> requires Entity::public_mutable_component_v<T>
    Result<std::reference_wrapper<T>> TryGetComponent();
    template <SomeComponent T>
    Result<std::reference_wrapper<const T>> TryGetComponent() const;

    template <SomeComponent...Ts> requires (Entity::public_mutable_component_v<Ts> && ...)
    std::tuple<Ts&...> GetComponents();
    template <SomeComponent...Ts>
    std::tuple<Ts&...> GetComponents(EntityPassKey);
    template <SomeComponent...Ts>
    std::tuple<const Ts&...> GetComponents() const;

    template <SomeComponent T> 
    bool HasComponent() const;
    template <SomeComponent...Ts> 
    bool HasComponents() const;

    // component visibility
    template <SomeComponent T> 
    bool GetComponentVisibility() const;
    template <SomeComponent...Ts> requires (Entity::public_mutable_component_v<Ts> && ...)
    void SetComponentVisibility(bool vis);

    // event production
    template <SomeEventData T>
    bool ShouldProduceEvent() const;
    template <SomeEventData T>
    void SetEventProduction(bool tf);

    // relations
    EntityRelations GetRelations();

    void Destroy();
    bool IsValid() const;
    Entity_t GetID() const { return id_; }

    bool operator==(const Entity& rhs) const { return id_ == rhs.id_; }
    bool operator==(const Entity_t& entT) const { return id_ == entT; }

protected:
    Entity_t id_ = kInvalidEntity;
    ECS* ecs_ = nullptr;
};

// ENTITY RELATIONS //
class EntityRelations : protected Entity
{
public:
    EntityRelations(Entity& ent) : Entity(ent) {}
    ~EntityRelations() = default;

    Entity AddChild();
    Entity AddChild(std::string_view childName);
    Entity AddProxyChild(); // to spoof extra copies of components for parent
    Entity AddProxyChild(std::string_view childName);

    bool IsParent() const;
    bool IsChild() const;

    bool IsParentOf(Entity_t child) const;
    bool IsParentOf(const Entity& child) const;

    bool IsChildOf(Entity_t parent) const;
    bool IsChildOf(const Entity& parent) const;

    bool HasChildren() const;

    Entity GetParent();
    std::vector<Entity> GetChildren();
    Entity FindChild(Entity_t childId);
    Entity FindChild(std::string_view childName);
};

// ECS //
class ECS
{
public:
    friend class Entity;
    friend class EntityRelations;

    ~ECS() = default;

    ECS(const ECS&) = delete;
    ECS(ECS&&) = delete;
    ECS& operator=(const ECS&) = delete;
    ECS& operator=(ECS&&) = delete;

    static Entity CreateEntity();

    static std::vector<Entity> GetAllActiveEntities()
    {
        auto& ecs = ECS::Get();

        auto active = ecs.entityManager_.GetActiveEntities();

        std::vector<Entity> entities{ active.size() };

        std::transform(active.begin(), active.end(), entities.begin(), [&ecs](Entity_t id) {
            return Entity{ id, ecs };
        });

        return entities;
    }

    template <typename...Ts, typename Filter>
    static std::vector<Entity> GetAllEntitiesWith(Filter&& filter)
    {
        auto& ecs = ECS::Get();

        return ecs.GetAllEntitiesWithInternalFiltered<Ts...>(std::forward<Filter>(filter));
    }

    // grabs all active entities with all of the desired components
    template <typename...Ts>
    static std::vector<Entity> GetAllEntitiesWith()
    {
        auto& ecs = ECS::Get();

        uint64_t withMask = (Ts::componentBit | ...);
      
        return ecs.GetAllEntitiesWithImpl(withMask, 
            [](uint64_t sig, uint64_t mask) -> bool { return (sig & mask) == mask; });
    }

    // grabs all active entities with at least one of the desired components
    template <typename...Ts>
    static std::vector<Entity> GetAllEntitiesWithAny()
    {
        auto& ecs = ECS::Get();

        uint64_t anyMask = (Ts::componentBit | ...);

        return ecs.GetAllEntitiesWithImpl(anyMask, 
            [](uint64_t sig, uint64_t mask) -> bool { return sig & mask; });
    }

    // grabs all active entities with signature matching requested components exactly
    // (ActiveState signature is added by default)
    template <typename...Ts>
    static std::vector<Entity> GetAllEntitiesWithOnly()
    {
        auto& ecs = ECS::Get();

        uint64_t exactMask = (ActiveState::componentBit | ... | Ts::componentBit);

        return ecs.GetAllEntitiesWithImpl(exactMask, 
            [](uint64_t sig, uint64_t mask) -> bool { return sig == mask; });
    }

    static Entity GetEntityByID(Entity_t id);

private:
    Entity_t CreateEntity_t();

    void DestroyEntity(Entity_t entity);

    template <SomeComponent T>
    T& AddComponent(Entity_t entity, T&& cmp)
    {
        return componentManager_.AddComponent<T>(entity, std::forward<T>(cmp));
    }

    template <SomeComponent T>
    T& AddComponent(Entity_t entity)
    {
        return componentManager_.AddComponent<T>(entity);
    }

    template <SomeComponent T>
    void RemoveComponent(Entity_t entity)
    {
        return componentManager_.RemoveComponent<T>(entity);
    }

    template <SomeComponent T>
    T& GetComponent(Entity_t entity)
    {
        return componentManager_.GetComponent<T>(entity);
    }

    template <SomeComponent T>
    const T& GetComponent(Entity_t entity) const 
    {
        return componentManager_.GetComponent<T>(entity);
    }

    template <SomeComponent...Ts>
    std::tuple<Ts&...> GetComponents(Entity_t entity)
    {
        return std::tie(componentManager_.GetComponent<Ts>(entity)...);
    }

    template <SomeComponent...Ts>
    std::tuple<const Ts&...> GetComponents(Entity_t entity) const
    {
        return std::tie(componentManager_.GetComponent<Ts>(entity)...);
    }

    template <SomeComponent T>
    bool HasComponent(Entity_t entity) const
    {
        return componentManager_.GetSignature(entity) & T::componentBit;
    }

    template <SomeComponent...Ts, typename Filter>
    std::vector<Entity> GetAllEntitiesWithInternalFiltered(Filter&& filter)
    {
        std::vector<Entity> result;
        const uint64_t mask = (Ts::componentBit | ...);

        auto activeEntities = entityManager_.GetActiveEntities();

        result.reserve(activeEntities.size());

        for (const auto& ent : activeEntities)
        {
            const uint64_t entitySig = componentManager_.GetSignature(ent);

            if ((entitySig & mask) != mask)
            {
                continue;
            }

            if constexpr (HasBooleanNotOperator<Filter>)
            {
                if (!filter)
                {
                    continue;
                }
            }

            if (!std::invoke(filter, componentManager_.GetComponent<Ts>(ent)...))
            {
                continue;
            }

            result.emplace_back(ent, *this);
        }

        return result;
    }

    std::vector<Entity> GetAllEntitiesWithImpl(uint64_t mask, bool(*testFn)(uint64_t, uint64_t))
    {
        auto activeEntities = entityManager_.GetActiveEntities();

        std::vector<Entity> result;
        result.reserve(activeEntities.size());

        for (const auto& entity : activeEntities)
        {
            // get full list of components that entity has
            uint64_t entitySig = componentManager_.GetSignature(entity);

            assert(componentManager_.HasComponent<EntityFlags>(entity));
            const auto& flags = componentManager_.GetComponent<EntityFlags>(entity);

            // remove bits for components marked invisible
            entitySig &= flags.componentVisibilityFlags;

            if (testFn(entitySig, mask))
            {
                result.emplace_back(entity, *this);
            }
        }

        return result;
    }

    //template <typename...Ts>
    //std::vector<Entity> GetAllEntitiesWithInternal()
    //{
    //    std::vector<Entity> result;
    //    uint64_t excludeMask = 0;

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

    //    const uint64_t includeMask = (makeMasks.template operator()<Ts>() | ...);

    //    auto activeEntities = entityManager_.GetActiveEntities();

    //    result.reserve(activeEntities.size());

    //    for (const auto& ent : activeEntities)
    //    {
    //        const uint64_t entitySig = componentManager_.GetSignature(ent);

    //        const auto& visibilityFlags = 
    //            componentManager_.GetComponent<EntityFlags>(ent).componentVisibilityFlags;

    //        if (((entitySig & includeMask) != includeMask) || (entitySig & excludeMask))
    //        { 
    //            continue;
    //        }

    //        result.emplace_back(ent, *this);
    //    }

    //    return result;
    //}

    //template <typename...Ts>
    //std::vector<Entity> GetAllEntitiesWithAnyInternal()
    //{
    //    uint64_t includeMask = (Ts::componentBit | ...);

    //    auto activeEntities = entityManager_.GetActiveEntities();

    //    std::vector<Entity> result;
    //    result.reserve(activeEntities.size()); 

    //    for (const auto& entity : activeEntities)
    //    {
    //        if (componentManager_.GetSignature(entity) & includeMask)
    //        {
    //            result.emplace_back(entity, *this);
    //        }  
    //    }

    //    return result;
    //}

    bool IsEntityActive(Entity_t entity) const;
    bool IsEntityValid(Entity_t entity) const;

    EntityManager& GetEntityManager() { return entityManager_; }
    const EntityManager& GetEntityManager() const { return entityManager_; }

    ComponentManager& GetComponentManager() { return componentManager_; }
    const ComponentManager& GetComponentManager() const { return componentManager_; }

    static ECS& Get();

    ECS() = default;

    EntityManager entityManager_;
    ComponentManager componentManager_;
};

// ENTITY DEFS //
template <SomeComponent T> requires Entity::public_mutable_component_v<T>
inline T& Entity::AddComponent(T&& cmp)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->AddComponent<T>(id_, std::forward<T>(cmp));
}

template <SomeComponent T> requires Entity::public_mutable_component_v<T>
inline T& Entity::AddComponent()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->AddComponent<T>(id_);
}

template<SomeComponent T>
inline T& Entity::AddComponent(T&& cmp, EntityPassKey)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->AddComponent<T>(id_, std::forward<T>(cmp));
}

template<SomeComponent T>
inline T& Entity::AddComponent(EntityPassKey)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->AddComponent<T>(id_);
}

template <SomeComponent T> requires Entity::public_mutable_component_v<T>
inline void Entity::RemoveComponent()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->RemoveComponent<T>(id_);
}

template<SomeComponent T>
inline void Entity::RemoveComponent(EntityPassKey)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->RemoveComponent<T>(id_);
}

template <SomeComponent T> requires Entity::public_mutable_component_v<T>
inline T& Entity::GetComponent()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponent<T>(id_);
}

template<SomeComponent T>
inline T& Entity::GetComponent(EntityPassKey)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponent<T>(id_);
}

template <SomeComponent T>
inline const T& Entity::GetComponent() const
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponent<T>(id_);
}

template<SomeComponent T> requires Entity::public_mutable_component_v<T>
inline Result<std::reference_wrapper<T>> Entity::TryGetComponent()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    if (!ecs_->HasComponent<T>(id_))
    {
        return MAKE_ERROR("entity did not have requested component");
    }
    
    return std::ref(ecs_->GetComponent<T>(id_));
}

template<SomeComponent T>
inline Result<std::reference_wrapper<const T>> Entity::TryGetComponent() const
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    if (!ecs_->HasComponent<T>(id_))
    {
        return MAKE_ERROR("entity did not have requested component");
    }

    return std::cref(ecs_->GetComponent<T>(id_));
}

template <SomeComponent...Ts> requires (Entity::public_mutable_component_v<Ts> && ...)
inline std::tuple<Ts&...> Entity::GetComponents()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponents<Ts...>(id_);
}

template<SomeComponent ...Ts>
inline std::tuple<Ts&...> Entity::GetComponents(EntityPassKey)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponents<Ts...>(id_);
}

template<SomeComponent ...Ts>
inline std::tuple<const Ts&...> Entity::GetComponents() const
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponents<Ts...>(id_);
}

template <SomeComponent T>
inline bool Entity::HasComponent() const
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->HasComponent<T>(id_);
}

template <SomeComponent...Ts>
inline bool Entity::HasComponents() const
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return (ecs_->HasComponent<Ts>(id_) && ...);
}

template <SomeComponent T>
inline bool Entity::GetComponentVisibility() const
{
    assert(HasComponent<EntityFlags>());

    const auto& visibilityFlags = ecs_->GetComponent<EntityFlags>(id_).componentVisibilityFlags;

    return visibilityFlags.Test<T>();
}

template <SomeComponent...Ts> requires (Entity::public_mutable_component_v<Ts> && ...)
inline void Entity::SetComponentVisibility(bool vis)
{
    assert(HasComponent<EntityFlags>());

    auto& visibilityFlags = ecs_->GetComponent<EntityFlags>(id_).componentVisibilityFlags;

    if constexpr (sizeof...(Ts) == 0) // set/unset all
    {
        // skips setting/unsetting visibility of publicly immutable components
        auto setVisibleComponents = [vis, &visibilityFlags]<typename...Cmps>() {
            (
                    (
                    public_mutable_component_v<Cmps>
                        ? visibilityFlags.Set<Cmps>(vis) 
                        : (void)0
                    ), 
            ...);
        };

        ComponentTypeList::Apply(setVisibleComponents);
    }
    else
    {
        visibilityFlags.Set<Ts...>(vis);
    }
}

template <SomeEventData T>
inline bool Entity::ShouldProduceEvent() const
{
    assert(HasComponent<EntityFlags>());

    const auto& eventProductionFlags = ecs_->GetComponent<EntityFlags>(id_).eventProductionFlags;

    return eventProductionFlags.Test<T>();
}

template <SomeEventData T>
inline void Entity::SetEventProduction(bool tf)
{
    assert(HasComponent<EntityFlags>());

    auto& eventProductionFlags = ecs_->GetComponent<EntityFlags>(id_).eventProductionFlags;

    eventProductionFlags.Set<T>(tf);
}

