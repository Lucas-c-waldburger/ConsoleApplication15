#pragma once
#include "EntityManager.h"
#include "ComponentManager.h"
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
    Entity() : id_(kInvalidEntity), ecs_(nullptr) {}
    Entity(Entity_t id, ECS& ecs) : id_(id), ecs_(&ecs) {}

    template <SomeComponent T> requires (!RelationalComponentType<T>) T& AddComponent(T cmp = {});
    template <SomeComponent T> requires (!RelationalComponentType<T>) void RemoveComponent();

    template <SomeComponent T> requires (!RelationalComponentType<T>) T& GetComponent();
    template <SomeComponent T> const T& GetComponent() const;

    template <SomeComponent...Ts> requires (!RelationalComponentType<Ts> && ...) std::tuple<Ts&...> GetComponents();

    template <SomeComponent T> bool HasComponent() const;
    template <SomeComponent...Ts> bool HasComponents() const;

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

    bool IsParent() const;
    bool IsChild() const;

    bool IsParentOf(Entity_t child) const;
    bool IsParentOf(const Entity& child) const;

    bool IsChildOf(Entity_t parent) const;
    bool IsChildOf(const Entity& parent) const;

    Entity GetParent();
    std::vector<Entity> GetChildren();
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

    static Entity CreateEntity()
    {
        auto& ecs = ECS::Get();

        return Entity{ ECS::Get().CreateEntity_t(), ecs };
    }

    template <typename...Ts, typename Filter>
    static std::vector<Entity> GetAllEntitiesWith(Filter&& filter)
    {
        auto& ecs = ECS::Get();

        return ecs.GetAllEntitiesWithInternalFiltered<Ts...>(std::forward<Filter>(filter));
    }

    template <typename...Ts>
    static std::vector<Entity> GetAllEntitiesWith()
    {
        auto& ecs = ECS::Get();

        return ecs.GetAllEntitiesWithInternal<Ts...>();
    }

    template <typename...Ts>
    static std::vector<Entity> GetAllEntitiesWithAny()
    {
        auto& ecs = ECS::Get();

        return ecs.GetAllEntitiesWithAnyInternal<Ts...>();
    }

    static Entity GetEntityByID(Entity_t id)
    {
        auto& ecs = ECS::Get();

        if (!ecs.IsEntityActive(id))
        {
            return Entity{ kInvalidEntity, ecs };
        }
        return Entity{ id, ecs };
    }

private:
    Entity_t CreateEntity_t();

    void DestroyEntity(Entity_t entity);

    template <SomeComponent T>
    T& AddComponent(Entity_t entity, T cmp = {})
    {
        return componentManager_.AddComponent<T>(entity, std::move(cmp));
    }

    template <RelationalComponentType T>
    T& AddComponent(Entity_t entity, T cmp)
    {
        return componentManager_.AddComponent<T>(entity, std::move(cmp));
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
    const T& GetComponent(Entity_t entity) const // all relationship stuff has to be done through relations API
    {
        return componentManager_.GetComponent<T>(entity);
    }

    template <SomeComponent...Ts>
    std::tuple<Ts&...> GetComponents(Entity_t entity)
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

    template <typename...Ts>
    std::vector<Entity> GetAllEntitiesWithInternal()
    {
        std::vector<Entity> result;
        uint64_t excludeMask = 0;

        auto makeMasks = [&excludeMask]<typename T>() -> uint64_t {
            if constexpr (is_exclude<T>::value)
            {
                excludeMask |= T::WrappedType::componentBit;
                return 0;
            }
            else
            {
                return T::componentBit;
            }
        };

        const uint64_t includeMask = (makeMasks.template operator()<Ts>() | ...);

        auto activeEntities = entityManager_.GetActiveEntities();

        result.reserve(activeEntities.size());

        for (const auto& ent : activeEntities)
        {
            const uint64_t entitySig = componentManager_.GetSignature(ent);

            if (((entitySig & includeMask) != includeMask) || (entitySig & excludeMask))
            {
                continue;
            }

            result.emplace_back(ent, *this);
        }

        return result;
    }

    template <typename...Ts>
    std::vector<Entity> GetAllEntitiesWithAnyInternal()
    {
        uint64_t includeMask = (Ts::componentBit | ...);

        auto activeEntities = entityManager_.GetActiveEntities();

        std::vector<Entity> result;
        result.reserve(activeEntities.size());

        for (const auto& entity : activeEntities)
        {
            if (componentManager_.GetSignature(entity) & includeMask)
            {
                result.emplace_back(entity, *this);
            }  
        }

        return result;
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
};

// ENTITY DEFS //
template <SomeComponent T> requires (!RelationalComponentType<T>)
inline T& Entity::AddComponent(T cmp)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->AddComponent<T>(id_, std::move(cmp));
}

template <SomeComponent T> requires (!RelationalComponentType<T>)
inline void Entity::RemoveComponent()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->RemoveComponent<T>(id_);
}

template <SomeComponent T> requires (!RelationalComponentType<T>)
inline T& Entity::GetComponent()
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

template<SomeComponent...Ts> requires (!RelationalComponentType<Ts> && ...)
inline std::tuple<Ts&...> Entity::GetComponents()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponents<Ts...>(id_);
}

template<SomeComponent T>
inline bool Entity::HasComponent() const
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->HasComponent<T>(id_);
}

template<SomeComponent...Ts>
inline bool Entity::HasComponents() const
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return (ecs_->HasComponent<Ts>(id_) && ...);
}

