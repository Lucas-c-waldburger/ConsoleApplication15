#pragma once
#include "EntityT.h"

class EntityManager;
class ComponentManager;

class EntityDestructor 
{
public:
	static void EntityDestroyed(EntityManager& entityManager, ComponentManager& componentManager, 
								Entity_t entityId);

private:
};