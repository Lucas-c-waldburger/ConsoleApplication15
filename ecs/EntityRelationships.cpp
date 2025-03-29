#include "EntityRelationships.h"
#include <cassert>

void EntityRelations::SetParent(impl::ComponentManager& componentManager, Entity_t entity, Entity_t requestedParent)
{
    // make sure we're not setting our current parent to one of our children
    if (componentManager.HasComponent<Children>(entity))
    {
        assert(!componentManager.GetComponent<Children>(entity).childEntities.contains(requestedParent));
    }

    auto& existingParentEntity = componentManager.AddComponent<Parent>(entity).parentEntity;

    if (existingParentEntity != kInvalidEntity) // changing from prev parent to another
    {
        assert(componentManager.HasComponent<Children>(existingParentEntity));

        auto& existingParentsChildren =
            componentManager.GetComponent<Children>(existingParentEntity).childEntities;

        assert(existingParentsChildren.contains(entity));

        existingParentsChildren.erase(entity);
    }

    if (requestedParent == kInvalidEntity) // no new parent, we're just removing 
    {
        componentManager.RemoveComponent<Parent>(entity);

        return;
    }

    if (componentManager.HasComponent<Parent>(requestedParent))
    {
        // avoid cyclical relation
        assert(componentManager.GetComponent<Parent>(requestedParent).parentEntity != entity);
    }

    existingParentEntity = requestedParent;

    auto& newParentsChildren = componentManager.AddComponent<Children>(requestedParent).childEntities;

    newParentsChildren.insert(entity);
}

bool EntityRelations::AddChild(impl::ComponentManager& componentManager, Entity_t entity, Entity_t requestedChild)
{
    // make sure we're not setting our current parent as child also
    if (componentManager.HasComponent<Parent>(entity))
    {
        assert(componentManager.GetComponent<Parent>(entity).parentEntity != requestedChild);
    }

    auto& childsCurrentParent = componentManager.AddComponent<Parent>(requestedChild).parentEntity;

    if (childsCurrentParent != kInvalidEntity) // child already has a different parent
    {
        return false;
    }

    childsCurrentParent = entity;

    auto& children = componentManager.AddComponent<Children>(entity).childEntities;

    children.insert(requestedChild);

    return true;
}

bool EntityRelations::RemoveChild(impl::ComponentManager& componentManager, Entity_t entity, Entity_t requestedChild)
{
    if (!componentManager.HasComponent<Children>(entity)) { return false; }

    auto& children = componentManager.GetComponent<Children>(entity).childEntities;

    if (!children.contains(requestedChild)) { return false; }

    children.erase(requestedChild);

    assert(componentManager.HasComponent<Parent>(requestedChild));

    auto& childsParent = componentManager.GetComponent<Parent>(requestedChild).parentEntity;

    assert(childsParent == entity);

    componentManager.RemoveComponent<Parent>(requestedChild);

    return true;
}

void EntityRelations::DestroyRelationshipsWithEntity(impl::ComponentManager& componentManager, Entity_t entity)
{
    if (componentManager.HasComponent<Parent>(entity))
    {
        Entity_t parent = componentManager.GetComponent<Parent>(entity).parentEntity;
        assert(componentManager.HasComponent<Children>(parent));

        auto& childrenOfParent = componentManager.GetComponent<Children>(parent).childEntities;
        assert(childrenOfParent.contains(entity));

        childrenOfParent.erase(entity);

        if (childrenOfParent.empty())
        {
            componentManager.RemoveComponent<Children>(parent);
        }
    }
    if (componentManager.HasComponent<Children>(entity))
    {
        auto& children = componentManager.GetComponent<Children>(entity).childEntities;
        for (auto& child : children)
        {
            assert(componentManager.HasComponent<Parent>(child));
            auto& parentOfChild = componentManager.GetComponent<Parent>(child).parentEntity;

            assert(entity == parentOfChild);

            componentManager.RemoveComponent<Parent>(child);
        }
    }
}