#pragma once
#include "ComponentManager.h"
#include "EntityManager.h"
#include "EntityT.h"
#include "../core/ReadOnly.h"

class EntityDestructor 
{
public:
	static void EntityDestroyed(EntityManager& entityManager, ComponentManager& componentManager, 
								Entity_t entityId);

private:
};