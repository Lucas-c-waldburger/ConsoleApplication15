//#pragma once
//#include "../components/ComponentConcepts.h"
//#include "../components/RelationComponents.h"
//#include "ECSImpl.h"
//
//class EntityRelations;
//
//class Entity
//{
//public:
//    Entity() : id_(kInvalidEntity), ecs_(nullptr) {}
//    Entity(Entity_t id, ECSImpl& ecs) : id_(id), ecs_(&ecs) {}
//
//    template <ComponentType T> requires (!RelationalComponentType<T>) T& AddComponent(T cmp = {});
//    template <ComponentType T> requires (!RelationalComponentType<T>) void RemoveComponent();
//
//    template <ComponentType T> requires (!RelationalComponentType<T>) T& GetComponent();
//    template <ComponentType T> const T& GetComponent() const;
//
//    template <ComponentType...Ts> requires (!RelationalComponentType<Ts> && ...) std::tuple<Ts&...> GetComponents();
//
//    template <ComponentType T> bool HasComponent() const;
//    template <ComponentType...Ts> bool HasComponents() const;
//
//    EntityRelations GetRelations();
//
//    void Destroy();
//    bool IsValid() const;
//    Entity_t GetID() const { return id_; }
//
//    bool operator==(const Entity& rhs) const { return id_ == rhs.id_; }
//    bool operator==(const Entity_t& entT) const { return id_ == entT; }
//
//protected:
//    Entity_t id_ = kInvalidEntity;
//    ECSImpl* ecs_ = nullptr;
//};
//
//class EntityRelations : protected Entity
//{
//public:
//    EntityRelations(Entity& ent) : Entity(ent) {}
//    ~EntityRelations() = default;
//
//    bool SetParent(const Entity& requestedParent);
//    bool RemoveParent();
//    bool AddChild(const Entity& requestedChild);
//    bool RemoveChild(const Entity& requestedChild);
//
//    bool IsParent() const;
//    bool IsChild() const;
//    Result<Entity> GetParent();
//    Result<std::vector<Entity>> GetChildren();
//
//    template <ComponentType T>
//    Result<std::vector<Entity>> GetChildrenWithSlaveComponent();
//};
//
//// ENTITY
//template <ComponentType T> requires (!RelationalComponentType<T>)
//inline T& Entity::AddComponent(T cmp)
//{
//    assert(ecs_);
//    assert(id_ != kInvalidEntity);
//
//    return ecs_->AddComponent<T>(id_, std::move(cmp));
//}
//
//template <ComponentType T> requires (!RelationalComponentType<T>)
//inline void Entity::RemoveComponent()
//{
//    assert(ecs_);
//    assert(id_ != kInvalidEntity);
//
//    return ecs_->RemoveComponent<T>(id_);
//}
//
//template <ComponentType T> requires (!RelationalComponentType<T>)
//inline T& Entity::GetComponent()
//{
//    assert(ecs_);
//    assert(id_ != kInvalidEntity);
//
//    return ecs_->GetComponent<T>(id_);
//}
//
//template <ComponentType T>
//inline const T& Entity::GetComponent() const
//{
//    assert(ecs_);
//    assert(id_ != kInvalidEntity);
//
//    return ecs_->GetComponent<T>(id_);
//}
//
//template<ComponentType ...Ts> requires (!RelationalComponentType<Ts> && ...)
//inline std::tuple<Ts&...> Entity::GetComponents()
//{
//    assert(ecs_);
//    assert(id_ != kInvalidEntity);
//
//    return ecs_->GetComponents<Ts...>(id_);
//}
//
//template<ComponentType T>
//inline bool Entity::HasComponent() const
//{
//    assert(ecs_);
//    assert(id_ != kInvalidEntity);
//
//    return ecs_->HasComponent<T>(id_);
//}
//
//template<ComponentType...Ts>
//inline bool Entity::HasComponents() const
//{
//    assert(ecs_);
//    assert(id_ != kInvalidEntity);
//
//    return (ecs_->HasComponent<Ts>(id_) && ...);
//}
//
//
//// ENTITY RELATIONS
//template <ComponentType T>
//Result<std::vector<Entity>> EntityRelations::GetChildrenWithSlaveComponent()
//{
//    assert(IsValid());
//
//    if (HasComponent<Parent>())
//    {
//        assert(!HasComponent<Children>());
//
//        return MAKE_ERROR("Entity is not a parent");
//    }
//
//    const auto& children = GetComponent<Children>().childEntities;
//
//    std::vector<Entity> slaveChildEntities;
//    slaveChildEntities.reserve(children.size());
//
//    for (const auto& child : children)
//    {
//        if ((T::componentBit & child.signature) == 0)
//        {
//            continue;
//        }
//
//        if (!ecs_->IsEntityActive(child.entityId))
//        {
//            continue;
//        }
//
//        Entity childEnt{ child.entityId, *ecs_ };
//
//        if (!childEnt.HasComponent<T>())
//        {
//            LOG_WARNING("Child had slave signature for this component but not holding component");
//            continue;
//        }
//
//        slaveChildEntities.push_back(childEnt);
//    }
//
//    return slaveChildEntities;
//}