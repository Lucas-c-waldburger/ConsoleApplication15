#pragma once
#include "ComponentManager.h"
#include "EntityManager.h"

//namespace impl {
//class ComponentManager;
//}

class EntityRelationsHelper
{
public:
    static bool IsParent(EntityManager& entityManager, impl::ComponentManager& componentManager, Entity_t entity);
    static bool IsChild(EntityManager& entityManager, impl::ComponentManager& componentManager, Entity_t entity);

    static bool IsParentOf(EntityManager& entityManager, impl::ComponentManager& componentManager, 
                           Entity_t parent, Entity_t child);
    static bool IsChildOf(EntityManager& entityManager, impl::ComponentManager& componentManager, 
                          Entity_t child, Entity_t parent);

    static Entity_t GetParent(EntityManager& entityManager, impl::ComponentManager& componentManager,
                              Entity_t child);
    static std::unordered_set<Entity_t>& GetChildren(EntityManager& entityManager, impl::ComponentManager& componentManager,
                                                     Entity_t parent);

    static Entity_t AddChild(EntityManager& entityManager, impl::ComponentManager& componentManager,
                             Entity_t parent);
    static void UnlinkChildFromParent(EntityManager& entityManager, impl::ComponentManager& componentManager,
                                      Entity_t child);

private:
};

