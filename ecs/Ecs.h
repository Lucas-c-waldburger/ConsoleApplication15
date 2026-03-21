#pragma once
#include "EntityConcepts.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include "ComponentMasks.h"
#include "EntityRelationsHelper.h"
#include "EntityAccess.h"
#include "../components/ComponentConcepts.h"
#include "../core/Logger.h"
#include "../user/UserComponentBridge.h"
#include <cassert>
#include <functional>

class ECS;
class EntityRelations;
class EntityEvents;
class EntityPhysics;

class B2World;
class EventBus2;

// ENTITY //
class Entity
{
private:
    template <typename TupLike>
    struct get_components_from_tuplike;

    template <template <typename...> class TupLike, typename...Ts>
    struct get_components_from_tuplike<TupLike<Ts...>>
    {
        using Ret = std::tuple<Ts&...>;
        using ConstRet = std::tuple<const Ts&...>;

        static Ret call(Entity&)
            requires (public_mutable_component_v<std::remove_cvref_t<Ts>> && ...);
        static ConstRet call_const(const Entity&);
        static Ret call_with_key(Entity&, EntityPassKey);
    };

public:
    Entity() : id_(kInvalidEntity), ecs_(nullptr) {}
    Entity(Entity_t id, ECS& ecs) : id_(id), ecs_(&ecs) {}

    template <typename T> 
        requires public_mutable_component_v<std::remove_cvref_t<T>>
    T& AddComponent(T&& cmp);
    template <typename T> requires public_mutable_component_v<T>
    T& AddComponent();
    template <typename T>
    T& AddComponent(T&& cmp, EntityPassKey);
    template <typename T>
    T& AddComponent(EntityPassKey);

    template <typename T> requires public_mutable_component_v<T>
    void RemoveComponent();
    template <typename T>
    void RemoveComponent(EntityPassKey);

    void ClearComponents();

    // component getters
    template <typename T> requires public_mutable_component_v<T>
    T& GetComponent();
    template <typename T>
    T& GetComponent(EntityPassKey);
    template <typename T>
    const T& GetComponent() const;

    template <typename T> requires public_mutable_component_v<T>
    Result<std::reference_wrapper<T>> TryGetComponent();
    template <typename T>
    Result<std::reference_wrapper<const T>> TryGetComponent() const;

    template <typename...Ts> requires (!(is_tuple_or_typelist_v<Ts> && ...) &&
                                        (public_mutable_component_v<Ts> && ...))
    std::tuple<Ts&...> GetComponents();
    template <typename...Ts> requires (!(is_tuple_or_typelist_v<Ts> && ...))
    std::tuple<Ts&...> GetComponents(EntityPassKey);
    template <typename...Ts> requires (!(is_tuple_or_typelist_v<Ts> && ...))
    std::tuple<const Ts&...> GetComponents() const;

    // getters from tuplike
    template <typename TupLike> requires (is_tuple_or_typelist_v<TupLike> && 
                                          all_public_mutable_components_v<TupLike>)
    typename get_components_from_tuplike<TupLike>::Ret GetComponents();
    template <typename TupLike> requires is_tuple_or_typelist_v<TupLike>
    typename get_components_from_tuplike<TupLike>::ConstRet GetComponents() const;
    template <typename TupLike> requires is_tuple_or_typelist_v<TupLike>
    typename get_components_from_tuplike<TupLike>::Ret GetComponents(EntityPassKey);

    // component testing
    template <typename T>
    bool HasComponent() const;
    template <typename...Ts>
    bool HasComponents() const;
    template <typename T, typename Fn> requires FnReturningBool<Fn, const T&>
    bool HasComponent(Fn&& fn) const;
    template <typename...Ts, typename Fn> requires FnReturningBool<Fn, const Ts&...>
    bool HasComponents(Fn&& fn) const;

    // component visibility
    template <typename T>
    bool GetComponentVisibility() const;
    template <typename...Ts> requires (public_mutable_component_v<Ts> && ...)
    void SetComponentVisibility(bool vis);

    // event production
    template <SomeEventData T>
    bool ShouldProduceEvent() const;
    template <SomeEventData T>
    void SetEventProduction(bool tf);

    // relations
    EntityRelations GetRelations();

    // events
    EntityEvents GetEvents(EventBus2& bus);

    // physics
    EntityPhysics GetPhysics(B2World& world);

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

    template <typename...Ts> requires (sizeof...(Ts) > 0)
    std::vector<Entity> GetAllChildrenWith();
    template <typename...Ts, typename Fn> requires (
        sizeof...(Ts) > 0 && std::is_invocable_r_v<bool, Fn, const Ts&...>)
    std::vector<Entity> GetAllChildrenWith(Fn&& fn);

    template <typename...Ts, typename Fn>
        requires (sizeof...(Ts) > 0 && std::invocable<Fn, Ts&...>)
    void ForAllChildrenWith(Fn&& fn);

    template <typename...Ts, typename Fn>
        requires (sizeof...(Ts) > 0 && std::invocable<Fn, const Ts&...>)
    void ForAllChildrenWith(Fn&& fn) const;
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

    template <typename...Ts, typename Fn>
        requires (sizeof...(Ts) > 0 && std::invocable<Fn, Ts&...>)
    static void ForAllEntitiesWith(Fn&& fn)
    {
        auto& ecs = ECS::Get();

        return ecs.ForAllEntitiesWithImpl<Ts...>(
            ecs.GetAllActiveEntities(), std::forward<Fn>(fn)
        );
        //auto entities = ecs.GetAllEntityTsWithImpl<Ts...>();

        //for (Entity_t e : entities)
        //{
        //    auto componentTup = ecs.GetComponents<Ts...>(e);

        //    std::apply([&](auto&...cmps) {
        //        std::invoke(fn, cmps...);
        //    }, componentTup);
        //}
    }

    template <typename...Ts, typename Fn>
        requires (sizeof...(Ts) > 0 && std::invocable<Fn, const Ts&...>)
    static void ForAllEntitiesWith(Fn&& fn)
    {
        const auto& ecs = ECS::Get();

        return ecs.ForAllEntitiesWithImpl<Ts...>(
            ecs.GetAllActiveEntities(), std::forward<Fn>(fn)
        );
        //auto entities = ecs.GetAllEntityTsWithImpl<Ts...>();

        //for (Entity_t e : entities)
        //{
        //    auto componentTup = ecs.GetComponents<Ts...>(e);

        //    std::apply([&](const auto&...cmps) {
        //        std::invoke(fn, cmps...);
        //    }, componentTup);
        //}
    }

    template <typename...Ts> requires (sizeof...(Ts) > 0)
    static std::vector<Entity> GetAllEntitiesWith()
    {
        auto& ecs = ECS::Get();

        return ecs.GetAllEntitiesWithImpl<Ts...>();
    }

    // grabs all active entities with at least one of the desired components
    template <typename...Ts>
    static std::vector<Entity> GetAllEntitiesWithAny()
    {
        auto& ecs = ECS::Get();

        uint64_t anyMask = (0ULL | ... | ecs.GetComponentBit<Ts>());

        return ecs.GetAllEntitiesWithImpl(anyMask, 
            [](uint64_t sig, uint64_t mask) -> bool { return sig & mask; });
    }

    // grabs all active entities with signature matching requested components exactly
    // (ActiveState signature is added by default)
    template <typename...Ts>
    static std::vector<Entity> GetAllEntitiesWithOnly()
    {
        auto& ecs = ECS::Get();

        uint64_t exactMask = (ActiveState::componentBit | EntityFlags::componentBit);
        exactMask |= (... | ecs.GetComponentBit<Ts>());

        return ecs.GetAllEntitiesWithImpl(exactMask, 
            [](uint64_t sig, uint64_t mask) -> bool { return sig == mask; });
    }

    template <typename T>
    static bool IsComponentRegistered()
    {
        if constexpr (SomeComponent<T>)
        {
            return true;
        }
        else
        {
            const auto& ecs = ECS::Get();

            return ecs.userComponentBridge_.IsComponentDataRegistered<T>();
        }
    }

    static Entity GetEntityByID(Entity_t id);

private:
    template <typename T>
    ComponentSignature GetComponentBit() const
    {
        if constexpr (SomeComponent<T>)
        {
            return T::componentBit;
        }
        else
        {
            return userComponentBridge_.GetComponentDataSignature<T>();
        }
    }

    Entity_t CreateEntity_t();

    void DestroyEntity(Entity_t entity);

    template <typename T>
    void HandleUpdateTrigger(Entity_t entity)
    {
        if constexpr (SomeUpdateTriggeringComponent<T>)
        {
            componentManager_.AddComponent<typename T::UpdateType>(entity);
        }
    }

    template <typename T>
    T& AddComponent(Entity_t entity, T&& cmp)
    {
        using cmp_type_t = std::remove_cvref_t<T>;

        if constexpr (SomeComponent<cmp_type_t>)
        {
            HandleUpdateTrigger<cmp_type_t>(entity);

            return componentManager_.AddComponent<cmp_type_t>(
                entity, std::forward<T>(cmp));
        }
        else
        {
            return userComponentBridge_.AddComponentData<cmp_type_t>(
                entity, componentManager_, std::forward<T>(cmp));
        }
    }

    template <typename T>
    T& AddComponent(Entity_t entity)
    {
        if constexpr (SomeComponent<T>)
        {
            HandleUpdateTrigger<T>(entity);

            return componentManager_.AddComponent<T>(entity);
        }
        else
        {
            return userComponentBridge_.AddComponentData<T>(
                entity, componentManager_);
        }
    }

    template <typename T>
    void RemoveComponent(Entity_t entity)
    {
        if constexpr (SomeComponent<T>)
        {
            return componentManager_.RemoveComponent<T>(entity);
        }
        else
        {
            return userComponentBridge_.RemoveComponentData<T>(
                entity, componentManager_);
        }
    }

    template <typename T>
    T& GetComponent(Entity_t entity)
    {
        if constexpr (SomeComponent<T>)
        {
            HandleUpdateTrigger<T>(entity);

            return componentManager_.GetComponent<T>(entity);
        }
        else
        {
            return userComponentBridge_.GetComponentData<T>(
                entity, componentManager_);
        }
    }

    template <typename T>
    const T& GetComponent(Entity_t entity) const 
    {
        if constexpr (SomeComponent<T>)
        {
            return componentManager_.GetComponent<T>(entity);
        }
        else
        {
            return userComponentBridge_.GetComponentData<T>(entity, componentManager_);
        }
    }

    template <typename...Ts>
    std::tuple<Ts&...> GetComponents(Entity_t entity)
    {
        auto get = [this]<typename T>(Entity_t e) -> T& {
            if constexpr (SomeComponent<T>)
            {
                return componentManager_.GetComponent<T>(e);
            }
            else
            {
                return userComponentBridge_.GetComponentData<T>(
                    e, componentManager_);
            }
        };

        return std::tie(get.template operator()<Ts>(entity)...);
    }

    template <typename...Ts>
    std::tuple<const Ts&...> GetComponents(Entity_t entity) const
    {
        auto get = [this]<typename T>(Entity_t e) -> const T& {
            if constexpr (SomeComponent<T>)
            {
                return componentManager_.GetComponent<T>(e);
            }
            else
            {
                return userComponentBridge_.GetComponentData<T>(
                    e, componentManager_);
            }
        };

        return std::tie(get.template operator()<Ts>(entity)...);
    }

    template <typename T>
    bool HasComponent(Entity_t entity) const
    {
        if constexpr (SomeComponent<T>)
        {
            return componentManager_.GetSignature(entity) & T::componentBit;
        }
        else
        {
            return userComponentBridge_.HasComponentData<T>(
                entity, componentManager_);
        }
    }

    //// TODO: Does this actually work since adding UserComponentBridge?
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

    template <typename...Ts, typename Container> requires (sizeof...(Ts) > 0)
    std::vector<Entity> GetAllEntitiesWithImpl(Container&& entities)
    {
        // calculate include/exclude/any masks
        auto componentMasks = ComponentMasks::template MakeMasks<Ts...>(
            userComponentBridge_);

        std::vector<Entity> result;
        result.reserve(entities.size());

        for (const auto& entity : entities)
        {
            // get full list of components that entity has
            uint64_t entitySig = componentManager_.GetSignature(entity);

            // unless the Get() call explicitly asks to include MarkDestroyed component, omit entity
            if (componentManager_.HasComponent<MarkedDestroyed>(entity) &&
                ((componentMasks.includeMask & MarkedDestroyed::componentBit) == 0))
            {
                continue;
            }

            assert(componentManager_.HasComponent<EntityFlags>(entity));

            // remove invisible components
            entitySig &= componentManager_.GetComponent<EntityFlags>(entity).
                componentVisibilityFlags;
          
            if (componentMasks.ShouldIncludeEntity(entitySig))
            {
                result.emplace_back(entity, *this);
            }
        }

        return result;
    }

    template <typename...Ts, typename Container, typename Fn> requires (
        sizeof...(Ts) > 0 && std::is_invocable_r_v<bool, Fn, const Ts&...>)
    std::vector<Entity> GetAllEntitiesWithImpl(Container&& entities, Fn&& fn)
    {
        // calculate and cache include/exclude/any masks
        auto componentMasks = ComponentMasks::template MakeMasks<Ts...>(
            userComponentBridge_);

        std::vector<Entity> result;
        result.reserve(entities.size());

        for (const auto& entity : entities)
        {
            // get full list of components that entity has
            uint64_t entitySig = componentManager_.GetSignature(entity);

            // unless the Get() call explicitly asks to include MarkDestroyed component, omit entity
            if (componentManager_.HasComponent<MarkedDestroyed>(entity) &&
                ((componentMasks.includeMask & MarkedDestroyed::componentBit) == 0))
            {
                continue;
            }

            assert(componentManager_.HasComponent<EntityFlags>(entity));

            // remove invisible components
            entitySig &= componentManager_.GetComponent<EntityFlags>(entity).
                componentVisibilityFlags;

            if (componentMasks.ShouldIncludeEntity(entitySig))
            {
                if (std::invoke(fn, GetComponent<Ts>(entity)...))
                {
                    result.emplace_back(entity, *this);
                }
            }
        }

        return result;
    }

    template <typename...Ts> requires (sizeof...(Ts) > 0)
    std::vector<Entity> GetAllEntitiesWithImpl()
    {
        return GetAllEntitiesWithImpl<Ts...>(entityManager_.GetActiveEntities());
    }

    template <typename...Ts> requires (sizeof...(Ts) > 0)
    std::vector<Entity_t> GetAllEntityTsWithImpl() const
    {
        // calculate and cache include/exclude/any masks
        static constexpr auto componentMasks =
            ComponentMasks::template MakeMasks<Ts...>(userComponentBridge_);

        auto activeEntities = entityManager_.GetActiveEntities();

        std::vector<Entity_t> result;
        result.reserve(activeEntities.size());

        for (const auto& entity : activeEntities)
        {
            // get full list of components that entity has
            uint64_t entitySig = componentManager_.GetSignature(entity);

            // unless the Get() call explicitly asks to include MarkDestroyed component, omit entity
            if (componentManager_.HasComponent<MarkedDestroyed>(entity) &&
                ((componentMasks.includeMask & MarkedDestroyed::componentBit) == 0))
            {
                continue;
            }

            assert(componentManager_.HasComponent<EntityFlags>(entity));

            // remove invisible components
            entitySig &= componentManager_.GetComponent<EntityFlags>(entity).
                componentVisibilityFlags;

            if (componentMasks.ShouldIncludeEntity(entitySig))
            {
                result.emplace_back(entity);
            }
        }

        return result;
    }

    template <typename...Ts, typename Container, typename Fn>
        requires (sizeof...(Ts) > 0 && std::invocable<Fn, Ts&...>)
    void ForAllEntitiesWithImpl(Container&& entities, Fn&& fn)
    {
        for (Entity_t e : entities)
        {
            assert(HasComponent<Ts>(e) && ...);
            auto componentTup = GetComponents<Ts...>(e);

            std::apply([&](auto&...cmps) {
                std::invoke(fn, cmps...);
            }, componentTup);
        }
    }

    template <typename...Ts, typename Container, typename Fn>
        requires (sizeof...(Ts) > 0 && std::invocable<Fn, const Ts&...>)
    void ForAllEntitiesWithImpl(Container&& entities, Fn&& fn) const
    {
        for (Entity_t e : entities)
        {
            assert(HasComponent<Ts>(e) && ...);
            auto componentTup = GetComponents<Ts...>(e);

            std::apply([&](const auto&...cmps) {
                std::invoke(fn, cmps...);
            }, componentTup);
        }
    }

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
    UserComponentBridge userComponentBridge_;
};

// ENTITY DEFS //
template <typename T> requires public_mutable_component_v<std::remove_cvref_t<T>>
inline T& Entity::AddComponent(T&& cmp)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->AddComponent<std::remove_cvref_t<T>>(id_, std::forward<T>(cmp));
}

template <typename T> requires public_mutable_component_v<T>
inline T& Entity::AddComponent()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->AddComponent<T>(id_);
}

template <typename T>
inline T& Entity::AddComponent(T&& cmp, EntityPassKey)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->AddComponent<std::remove_cvref_t<T>>(id_, std::forward<T>(cmp));
}

template <typename T>
inline T& Entity::AddComponent(EntityPassKey)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->AddComponent<T>(id_);
}

template <typename T> requires public_mutable_component_v<T>
inline void Entity::RemoveComponent()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->RemoveComponent<T>(id_);
}

template <typename T>
inline void Entity::RemoveComponent(EntityPassKey)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->RemoveComponent<T>(id_);
}

template <typename T> requires public_mutable_component_v<T>
inline T& Entity::GetComponent()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponent<T>(id_);
}

template <typename T>
inline T& Entity::GetComponent(EntityPassKey)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponent<T>(id_);
}

template <typename T>
inline const T& Entity::GetComponent() const
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponent<T>(id_);
}

template <typename T> requires public_mutable_component_v<T>
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

template <typename T>
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

template <typename...Ts> requires (!(is_tuple_or_typelist_v<Ts> && ...) &&
                                    (public_mutable_component_v<Ts> && ...))
inline std::tuple<Ts&...> Entity::GetComponents()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponents<Ts...>(id_);
}

template <typename...Ts> requires (!(is_tuple_or_typelist_v<Ts> && ...))
inline std::tuple<Ts&...> Entity::GetComponents(EntityPassKey)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponents<Ts...>(id_);
}

template <typename...Ts> requires (!(is_tuple_or_typelist_v<Ts> && ...))
inline std::tuple<const Ts&...> Entity::GetComponents() const
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponents<Ts...>(id_);
}

/* Get components via tup-like */
template <typename TupLike>
    requires (is_tuple_or_typelist_v<TupLike> && all_public_mutable_components_v<TupLike>)
typename Entity::get_components_from_tuplike<TupLike>::Ret 
Entity::GetComponents()
{
    return Entity::get_components_from_tuplike<TupLike>::call(*this);
}

template <typename TupLike> requires is_tuple_or_typelist_v<TupLike>
typename Entity::get_components_from_tuplike<TupLike>::ConstRet 
Entity::GetComponents() const
{
    return Entity::get_components_from_tuplike<TupLike>::call_const(*this);
}

template <typename TupLike> requires is_tuple_or_typelist_v<TupLike>
typename Entity::get_components_from_tuplike<TupLike>::Ret 
Entity::GetComponents(EntityPassKey k)
{
    return Entity::get_components_from_tuplike<TupLike>::call_with_key(*this, k);
}
/**/

template <typename T>
inline bool Entity::HasComponent() const
{
    return IsValid() && ecs_->HasComponent<T>(id_);
}

template <typename...Ts>
inline bool Entity::HasComponents() const
{
    return IsValid() && (ecs_->HasComponent<Ts>(id_) && ...);
}

template <typename T, typename Fn> requires FnReturningBool<Fn, const T&>
bool Entity::HasComponent(Fn&& fn) const
{
    return HasComponent<T>() &&
           std::invoke(std::forward<Fn>(fn), GetComponent<T>());
}

template <typename...Ts, typename Fn> requires FnReturningBool<Fn, const Ts&...>
bool Entity::HasComponents(Fn&& fn) const
{
    return HasComponents<Ts...>() &&
           std::invoke(std::forward<Fn>(fn), GetComponent<Ts>()...);
}

template <typename T>
inline bool Entity::GetComponentVisibility() const
{
    assert(HasComponent<EntityFlags>());

    const auto& visibilityFlags = 
        ecs_->GetComponent<EntityFlags>(id_).componentVisibilityFlags;

    return visibilityFlags.Test<T>();
}

template <typename...Ts> requires (public_mutable_component_v<Ts> && ...)
inline void Entity::SetComponentVisibility(bool vis)
{
    assert(HasComponent<EntityFlags>());

    auto& visibilityFlags = 
        ecs_->GetComponent<EntityFlags>(id_).componentVisibilityFlags;

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

// GET COMPONENTS FROM TUPLIKE
template <template <typename...> class TupLike, typename...Ts>
std::tuple<Ts&...>
Entity::get_components_from_tuplike<TupLike<Ts...>>::call(Entity& e)
    requires (public_mutable_component_v<std::remove_cvref_t<Ts>> && ...)
{
    return e.GetComponents<Ts...>();
}

template <template <typename...> class TupLike, typename...Ts>
std::tuple<const Ts&...>
Entity::get_components_from_tuplike<TupLike<Ts...>>::call_const(const Entity& e)
{
    return e.GetComponents<Ts...>();
}

template <template <typename...> class TupLike, typename...Ts>
std::tuple<Ts&...>
Entity::get_components_from_tuplike<TupLike<Ts...>>::call_with_key(Entity& e, 
                                                                   EntityPassKey k)
{
    return e.GetComponents<Ts...>(k);
}

// ENTITY RELATIONS
template <typename...Ts, typename Fn>
    requires (sizeof...(Ts) > 0 && std::invocable<Fn, Ts&...>)
void EntityRelations::ForAllChildrenWith(Fn&& fn)
{
    if (!(IsValid() && IsParent()))
    {
        return;
    }
  
    const auto& children = EntityRelationsHelper::GetChildren(
        ecs_->GetEntityManager(), ecs_->GetComponentManager(), id_);
  
    if (children.empty())
    {
        return;
    }

    return ecs_->ForAllEntitiesWithImpl<Ts...>(children, std::forward<Fn>(fn));
}

template <typename...Ts, typename Fn>
    requires (sizeof...(Ts) > 0 && std::invocable<Fn, const Ts&...>)
void EntityRelations::ForAllChildrenWith(Fn&& fn) const
{
    if (!(IsValid() && IsParent()))
    {
        return;
    }

    const auto& children = EntityRelationsHelper::GetChildren(
        ecs_->GetEntityManager(), ecs_->GetComponentManager(), id_);

    if (children.empty())
    {
        return;
    }

    const auto& cEcs = *ecs_;

    return cEcs.ForAllEntitiesWithImpl<Ts...>(children, std::forward<Fn>(fn));
}
 

template <typename...Ts> requires (sizeof...(Ts) > 0)
std::vector<Entity> EntityRelations::GetAllChildrenWith()
{
    if (!(IsValid() && IsParent()))
    {
        return {};
    }

    const auto& children = EntityRelationsHelper::GetChildren(
        ecs_->GetEntityManager(), ecs_->GetComponentManager(), id_);

    if (children.empty())
    {
        return {};
    }

    return ecs_->GetAllEntitiesWithImpl<Ts...>(children);
}

template <typename...Ts, typename Fn> requires (
    sizeof...(Ts) > 0 && std::is_invocable_r_v<bool, Fn, const Ts&...>)
std::vector<Entity> EntityRelations::GetAllChildrenWith(Fn&& fn)
{
    if (!(IsValid() && IsParent()))
    {
        return {};
    }

    const auto& children = EntityRelationsHelper::GetChildren(
        ecs_->GetEntityManager(), ecs_->GetComponentManager(), id_);

    if (children.empty())
    {
        return {};
    }

    return ecs_->GetAllEntitiesWithImpl<Ts...>(children, std::forward<Fn>(fn));
}