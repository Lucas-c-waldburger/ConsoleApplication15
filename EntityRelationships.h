#pragma once
#include "ComponentManager.h"

// static functions that coordinate adding/removing relationships among Parent/Child components of entities
class EntityRelations
{
public:
    static void SetParent(ComponentManager& componentManager, Entity_t entity, Entity_t requestedParent)
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

    static bool AddChild(ComponentManager& componentManager, Entity_t entity, Entity_t requestedChild)
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

    static bool RemoveChild(ComponentManager& componentManager, Entity_t entity, Entity_t requestedChild)
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

    static void DestroyRelationshipsWithEntity(ComponentManager& componentManager, Entity_t entity)
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

private:
    bool CheckNoCyclicalRelationshipUpImpl(ComponentManager& componentManager, Entity_t target, Entity_t current)
    {
        if (!componentManager.HasComponent<Parent>(current))
        {
            return true;
        }

        const Entity_t parent = componentManager.GetComponent<Parent>(current).parentEntity;

        if (parent == target)
        {
            return false;
        }

        return CheckNoCyclicalRelationshipUpImpl(componentManager, target, parent);
    }

    bool CheckNoCyclicalRelationshipDownImpl(ComponentManager& componentManager, Entity_t target, Entity_t current)
    {
        if (!componentManager.HasComponent<Children>(current))
        {
            return true;
        }

        const auto& children = componentManager.GetComponent<Children>(current).childEntities;

        if (children.contains(target)) // parent cant be child of its child
        {
            return false;
        }

        for (const auto child : children)
        {
            if (!CheckNoCyclicalRelationshipDownImpl(componentManager, target, child))
            {
                return false;
            }
        }

        return true;
    }

    bool CheckNoCyclicalRelationshipDown(ComponentManager& componentManager, Entity_t target)
    {
        return CheckNoCyclicalRelationshipDownImpl(componentManager, target, target);
    }

    bool CheckNoCyclicalRelationshipUp(ComponentManager& componentManager, Entity_t target)
    {
        return CheckNoCyclicalRelationshipUpImpl(componentManager, target, target);
    }
};

