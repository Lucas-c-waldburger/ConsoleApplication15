#pragma once
#include "EntityManager.h"
#include "EntityRelationships.h"
#include <cassert>
#include <functional>

// TODO: Do we need an ActiveComponent if we can just get that info directly from EntityManager?

class ECS;

class Entity
{
public:
    Entity(Entity_t id, ECS& ecs) : id_(id), ecs_(&ecs) {}

    template <ComponentType T> requires (!RelationalComponentType<T>) T& AddComponent(T cmp = {});
    template <ComponentType T> requires (!RelationalComponentType<T>) void RemoveComponent();

    template <ComponentType T> requires (!RelationalComponentType<T>) T& GetComponent();
    template <ComponentType T> const T& GetComponent() const;

    template <ComponentType...Ts> requires (!RelationalComponentType<Ts> && ...) std::tuple<Ts&...> GetComponents();

    template <ComponentType T> bool HasComponent() const;
    template <ComponentType...Ts> bool HasComponents() const;

    void SetParent(const Entity& requestedParent);
    void RemoveParent();
    bool AddChild(const Entity& requestedChild);
    bool RemoveChild(const Entity& requestedChild);

    void Destroy();
    bool IsValid() const;

    bool operator==(const Entity& rhs) const { return id_ == rhs.id_; }
    bool operator==(const Entity_t& entT) const { return id_ == entT; }

private:
    Entity_t id_ = kInvalidEntity;
    ECS* ecs_ = nullptr;
};

template <ComponentType T>
using ComponentFilter = bool(*)(const T&);
//using ComponentFilter = std::function<bool(const T&)>;

class ECS
{
public:
    friend class Entity;

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

    template <ComponentType T>
    static std::vector<Entity> GetAllEntitiesWith(ComponentFilter<T>&& filter)
    {
        auto& ecs = ECS::Get();

        return ecs.GetAllEntitiesWithInternal<T>(std::forward<ComponentFilter<T>>(filter));
    }

    template <typename...Ts>
    static std::vector<Entity> GetAllEntitiesWith()
    {
        auto& ecs = ECS::Get();

        return ecs.GetAllEntitiesWithInternal<Ts...>();
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
    Entity_t CreateEntity_t()
    {
        Entity_t entity = entityManager_.CreateEntity();
        componentManager_.EntityCreated(entity);

        return entity;
    }

    void DestroyEntity(Entity_t entity)
    {
        entityManager_.DestroyEntity(entity);
        EntityRelations::DestroyRelationshipsWithEntity(componentManager_, entity);
        componentManager_.EntityDestroyed(entity);
    }

    template <ComponentType T> requires (!RelationalComponentType<T>)
    T& AddComponent(Entity_t entity, T cmp = {})
    {
        return componentManager_.AddComponent<T>(entity, std::move(cmp));
    }

    template <RelationalComponentType T>
    T& AddComponent(Entity_t entity, T cmp)
    {
        return componentManager_.AddComponent<T>(entity, std::move(cmp));
    }

    template <ComponentType T> requires (!RelationalComponentType<T>)
        void RemoveComponent(Entity_t entity)
    {
        return componentManager_.RemoveComponent<T>(entity);
    }

    template <ComponentType T> requires (!RelationalComponentType<T>)
    T& GetComponent(Entity_t entity)
    {
        return componentManager_.GetComponent<T>(entity);
    }

    template <ComponentType T>
    const T& GetComponent(Entity_t entity) const // all relationship stuff has to be done through relations API
    {
        return componentManager_.GetComponent<T>(entity);
    }

    template <ComponentType...Ts> requires (!RelationalComponentType<Ts> && ...)
    std::tuple<Ts&...> GetComponents(Entity_t entity)
    {
        return std::tie(componentManager_.GetComponent<Ts>(entity)...);
    }

    template <ComponentType T>
    bool HasComponent(Entity_t entity) const
    {
        return componentManager_.GetSignature(entity) & T::componentBit;
    }

    template <typename T>
    std::vector<Entity> GetAllEntitiesWithInternal(ComponentFilter<T>&& filter)
    {
        std::vector<Entity> result;

        auto activeEntities = entityManager_.GetActiveEntities();
        for (const auto& ent : activeEntities)
        {
            const uint64_t entitySig = componentManager_.GetSignature(ent);

            if ((entitySig & T::componentBit) == 0)
            {
                continue;
            }

            if (!(filter && filter(componentManager_.GetComponent<T>(ent))))
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

        const uint64_t includeMask = (makeMasks.template operator() < Ts > () | ...);

        auto activeEntities = entityManager_.GetActiveEntities();
        for (const auto& ent : activeEntities)
        {
            const uint64_t entitySig = componentManager_.GetSignature(ent);

            if (((entitySig & includeMask) == 0) || (entitySig & excludeMask))
            {
                continue;
            }

            result.emplace_back(ent, *this);
        }

        return result;
    }

    bool IsEntityActive(Entity_t entity) const
    {
        assert(entity < kMaxEntities);

        bool activeAccordingToComponentManager = 
            componentManager_.GetSignature(entity) & ActiveState::componentBit;
        bool activeAccordingToEntityManager = entityManager_.IsEntityActive(entity);

        assert(activeAccordingToComponentManager == activeAccordingToEntityManager);

        return activeAccordingToComponentManager;
    }

    void SetParent(Entity_t entity, Entity_t requestedParent)
    {
        if (!IsEntityActive(entity) || !IsEntityActive(requestedParent)) { return; }

        return EntityRelations::SetParent(componentManager_, entity, requestedParent);
    }

    void RemoveParent(Entity_t entity)
    {
        if (!IsEntityActive(entity)) { return; }

        return EntityRelations::SetParent(componentManager_, entity, kInvalidEntity);
    }

    bool AddChild(Entity_t entity, Entity_t requestedChild)
    {
        if (!IsEntityActive(entity) || !IsEntityActive(requestedChild)) { return false; }

        return EntityRelations::AddChild(componentManager_, entity, requestedChild);
    }

    bool RemoveChild(Entity_t entity, Entity_t requestedChild)
    {
        if (!IsEntityActive(entity) || !IsEntityActive(requestedChild)) { return false; }

        return EntityRelations::RemoveChild(componentManager_, entity, requestedChild);
    }

    static ECS& Get()
    {
        static std::unique_ptr<ECS> ecs;
        if (!ecs)
        {
            ecs = std::unique_ptr<ECS>(new ECS());
        }

        return *ecs;
    }

    ECS() = default;

    EntityManager entityManager_;
    impl::ComponentManager componentManager_;
};

template <ComponentType T> requires (!RelationalComponentType<T>)
inline T& Entity::AddComponent(T cmp)
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->AddComponent<T>(id_, std::move(cmp));
}

template <ComponentType T> requires (!RelationalComponentType<T>)
inline void Entity::RemoveComponent()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->RemoveComponent<T>(id_);
}

template <ComponentType T> requires (!RelationalComponentType<T>)
inline T& Entity::GetComponent()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponent<T>(id_);
}

template <ComponentType T>
inline const T& Entity::GetComponent() const
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponent<T>(id_);
}

template<ComponentType ...Ts> requires (!RelationalComponentType<Ts> && ...)
inline std::tuple<Ts&...> Entity::GetComponents()
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->GetComponents<Ts...>(id_);
}

template<ComponentType T>
inline bool Entity::HasComponent() const
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return ecs_->HasComponent<T>(id_);
}

template<ComponentType ...Ts>
inline bool Entity::HasComponents() const
{
    assert(ecs_);
    assert(id_ != kInvalidEntity);

    return (ecs_->HasComponent<Ts>(id_) && ...);
}


