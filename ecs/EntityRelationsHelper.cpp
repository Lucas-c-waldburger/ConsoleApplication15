#include "EntityRelationsHelper.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include <cassert>

bool EntityRelationsHelper::IsParent(EntityManager& entityManager, ComponentManager& componentManager, Entity_t entity)
{
    if (entity == kInvalidEntity || !entityManager.IsEntityActive(entity))
    {
        return false;
    }

    return componentManager.HasComponent<Children>(entity);
}

bool EntityRelationsHelper::IsChild(EntityManager& entityManager, ComponentManager& componentManager, Entity_t entity)
{
    if (entity == kInvalidEntity || !entityManager.IsEntityActive(entity))
    {
        return false;
    }

    return componentManager.HasComponent<Parent>(entity);
}

bool EntityRelationsHelper::IsChildOf(EntityManager& entityManager, ComponentManager& componentManager, 
                                      Entity_t child, Entity_t parent)
{
    return IsParentOf(entityManager, componentManager, parent, child);
}

bool EntityRelationsHelper::IsParentOf(EntityManager& entityManager, ComponentManager& componentManager, 
                                       Entity_t parent, Entity_t child)
{
    if (!IsParent(entityManager, componentManager, parent))
    {
        return false;
    }
    if (!IsChild(entityManager, componentManager, child))
    {
        return false;
    }

    auto& parentsChildren = componentManager.GetComponent<Children>(parent).childEntityIds;
    if (!parentsChildren.contains(child))
    {
        return false;
    }

    assert(componentManager.GetComponent<Parent>(child).entityId == parent);

    return true;
}

Entity_t EntityRelationsHelper::GetParent(EntityManager& entityManager, 
                                          ComponentManager& componentManager, 
                                          Entity_t child)
{
    assert(IsChild(entityManager, componentManager, child));

    return componentManager.GetComponent<Parent>(child).entityId;
}

std::unordered_set<Entity_t>& EntityRelationsHelper::GetChildren(EntityManager& entityManager, 
                                                                 ComponentManager& componentManager, 
                                                                 Entity_t parent)
{
    assert(IsParent(entityManager, componentManager, parent));

    return componentManager.GetComponent<Children>(parent).childEntityIds;
}

Entity_t EntityRelationsHelper::AddChild(EntityManager& entityManager, ComponentManager& componentManager, 
                                         Entity_t parent)
{
    assert(IsParent(entityManager, componentManager, parent) || !IsChild(entityManager, componentManager, parent));

    auto& children = componentManager.AddComponent<Children>(parent).childEntityIds;
    
    Entity_t newChild = entityManager.CreateEntity();
    componentManager.EntityCreated(newChild);

    componentManager.AddComponent<Parent>(newChild).entityId = parent;
    children.insert(newChild);

    componentManager.AddComponent<EntityFlags>(newChild);

    return newChild;
}

void EntityRelationsHelper::UnlinkChildFromParent(EntityManager& entityManager, ComponentManager& componentManager,
                                                  Entity_t child)
{
    Entity_t parent = GetParent(entityManager, componentManager, child);

    auto& parentsChildren = GetChildren(entityManager, componentManager, parent);
    assert(parentsChildren.contains(child));

    parentsChildren.erase(child);

    //if (parentsChildren.empty())
    //{
    //    componentManager.RemoveComponent<Children>(parent);
    //}
}


//bool EntityRelationsHelper::SetParent(impl::ComponentManager& componentManager, Entity_t entity, Entity_t requestedParent)
//{
//    if (componentManager.HasComponent<Children>(entity))
//    {
//        // entity can't become a child if already a parent to children
//        return false;
//    }
//
//    if (requestedParent == kInvalidEntity) // means we're just clearing the parent component
//    {
//        componentManager.RemoveComponent<Parent>(entity);
//
//        return true;
//    }
//
//    if (componentManager.HasComponent<Parent>(requestedParent))
//    {
//        // requested parent is a child itself, can't promote it to a parent of this ent
//        return false;
//    }
//
//    auto& existingParentEntity = componentManager.AddComponent<Parent>(entity).entityId;
//
//    if (existingParentEntity != kInvalidEntity) // changing from prev parent to another
//    {
//        assert(componentManager.HasComponent<Children>(existingParentEntity));
//
//        auto& existingParentsChildren =
//            componentManager.GetComponent<Children>(existingParentEntity).childEntityIds;
//
//        assert(existingParentsChildren.contains(entity));
//
//        existingParentsChildren.erase(entity);
//    }
//
//    // set this entity's parent to requested one, add it to that parent's children
//    existingParentEntity = requestedParent;
//
//    auto& newParentsChildren = componentManager.AddComponent<Children>(requestedParent).childEntityIds;
//
//    // entity shouldn't already be in here
//    assert(!newParentsChildren.contains(entity));
//
//    newParentsChildren.insert(entity);
//
//    return true;
//}
//
//bool EntityRelationsHelper::AddChild(impl::ComponentManager& componentManager, Entity_t entity, Entity_t requestedChild)
//{
//    if (componentManager.HasComponent<Parent>(entity))
//    {
//        // entity can't become a parent if already a child to some other entity
//        return false;
//    }
//
//    if (requestedChild == kInvalidEntity)
//    {
//        return false;
//    }
//
//    if (componentManager.HasComponent<Children>(requestedChild))
//    {
//        // requested child is a parent itself, can't make it a child of this entity
//        return false;
//    }
//
//    // either retrieves current parent or constructs new component
//    auto& childsCurrentParent = componentManager.AddComponent<Parent>(requestedChild).entityId;
//
//    if (childsCurrentParent != kInvalidEntity) // child already has a different parent, can't steal it
//    {
//        return false;
//    }
//
//    childsCurrentParent = entity;
//
//    auto& children = componentManager.AddComponent<Children>(entity).childEntityIds;
//
//    children.insert(requestedChild);
//
//    return true;
//}
//
//bool EntityRelationsHelper::RemoveChild(impl::ComponentManager& componentManager, Entity_t entity, Entity_t requestedChild)
//{
//    if (!componentManager.HasComponent<Children>(entity)) { return false; }
//
//    auto& children = componentManager.GetComponent<Children>(entity).childEntityIds;
//
//    if (!children.contains(requestedChild)) { return false; }
//
//    children.erase(requestedChild);
//
//    assert(componentManager.HasComponent<Parent>(requestedChild));
//
//    auto& childsParent = componentManager.GetComponent<Parent>(requestedChild).entityId;
//
//    assert(childsParent == entity);
//
//    componentManager.RemoveComponent<Parent>(requestedChild);
//
//    return true;
//}
//
//void EntityRelationsHelper::DestroyRelationshipsWithEntity(impl::ComponentManager& componentManager, Entity_t entity)
//{
//    if (componentManager.HasComponent<Parent>(entity))
//    {
//        Entity_t parent = componentManager.GetComponent<Parent>(entity).entityId;
//        assert(componentManager.HasComponent<Children>(parent));
//
//        auto& childrenOfParent = componentManager.GetComponent<Children>(parent).childEntityIds;
//        assert(childrenOfParent.contains(entity));
//
//        childrenOfParent.erase(entity);
//
//        if (childrenOfParent.empty())
//        {
//            componentManager.RemoveComponent<Children>(parent);
//        }
//    }
//    if (componentManager.HasComponent<Children>(entity))
//    {
//        auto& children = componentManager.GetComponent<Children>(entity).childEntityIds;
//        for (auto child : children)
//        {
//            assert(componentManager.HasComponent<Parent>(child));
//            auto& parentOfChild = componentManager.GetComponent<Parent>(child).entityId;
//
//            assert(entity == parentOfChild);
//
//            componentManager.RemoveComponent<Parent>(child);
//        }
//    }
//}