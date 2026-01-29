#pragma once
#include <unordered_set>
#include <set>
#include "EntityT.h"
//#include "EntityManager.h"
//#include "ComponentManager.h"

class EntityManager;
class ComponentManager;

class EntityRelationsHelper
{
public:
    static bool IsParent(EntityManager& entityManager, ComponentManager& componentManager, Entity_t entity);
    static bool IsChild(EntityManager& entityManager, ComponentManager& componentManager, Entity_t entity);

    static bool IsParentOf(EntityManager& entityManager, ComponentManager& componentManager, 
                           Entity_t parent, Entity_t child);
    static bool IsChildOf(EntityManager& entityManager, ComponentManager& componentManager, 
                          Entity_t child, Entity_t parent);

    static Entity_t GetParent(EntityManager& entityManager, ComponentManager& componentManager,
                              Entity_t child);
    static std::set<Entity_t>& GetChildren(EntityManager& entityManager, 
                                           ComponentManager& componentManager,
                                           Entity_t parent);

    static Entity_t AddChild(EntityManager& entityManager, ComponentManager& componentManager,
                             Entity_t parent);

    static void UnlinkChildFromParent(EntityManager& entityManager, ComponentManager& componentManager,
                                      Entity_t child);

private:
};

