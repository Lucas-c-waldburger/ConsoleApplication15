#pragma once
#include "ComponentManager.h"

class EntityRelations
{
public:
    static void SetParent(impl::ComponentManager& componentManager, Entity_t entity, Entity_t requestedParent);
    static bool AddChild(impl::ComponentManager& componentManager, Entity_t entity, Entity_t requestedChild);
    static bool RemoveChild(impl::ComponentManager& componentManager, Entity_t entity, Entity_t requestedChild);
    static void DestroyRelationshipsWithEntity(impl::ComponentManager& componentManager, Entity_t entity);

private:
    //bool CheckNoCyclicalRelationshipUpImpl(ComponentManager& componentManager, Entity_t target, Entity_t current)
    //{
    //    if (!componentManager.HasComponent<Parent>(current))
    //    {
    //        return true;
    //    }

    //    const Entity_t parent = componentManager.GetComponent<Parent>(current).parentEntity;

    //    if (parent == target)
    //    {
    //        return false;
    //    }

    //    return CheckNoCyclicalRelationshipUpImpl(componentManager, target, parent);
    //}

    //bool CheckNoCyclicalRelationshipDownImpl(ComponentManager& componentManager, Entity_t target, Entity_t current)
    //{
    //    if (!componentManager.HasComponent<Children>(current))
    //    {
    //        return true;
    //    }

    //    const auto& children = componentManager.GetComponent<Children>(current).childEntities;

    //    if (children.contains(target)) // parent cant be child of its child
    //    {
    //        return false;
    //    }

    //    for (const auto child : children)
    //    {
    //        if (!CheckNoCyclicalRelationshipDownImpl(componentManager, target, child))
    //        {
    //            return false;
    //        }
    //    }

    //    return true;
    //}

    //bool CheckNoCyclicalRelationshipDown(ComponentManager& componentManager, Entity_t target)
    //{
    //    return CheckNoCyclicalRelationshipDownImpl(componentManager, target, target);
    //}

    //bool CheckNoCyclicalRelationshipUp(ComponentManager& componentManager, Entity_t target)
    //{
    //    return CheckNoCyclicalRelationshipUpImpl(componentManager, target, target);
    //}
};

