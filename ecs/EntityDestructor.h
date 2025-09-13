#pragma once
#include "EntityT.h"

class EntityManager;
class ComponentManager;
//class EventBus2;

// TODO: Pull all this out into a separate thing
class EntityDestructor 
{
public:
	static void EntityDestroyed(EntityManager& entityManager, ComponentManager& componentManager, 
								/*EventBus2& bus,*/ Entity_t entityId);

private:
}; 