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

std::set<Entity_t>& EntityRelationsHelper::GetChildren(EntityManager& entityManager, 
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
}


