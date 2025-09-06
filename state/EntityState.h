#pragma once
#include "../core/CatTypeID.h"

class Entity;

class EntityState
{
public:
	virtual ~EntityState() = 0;
	virtual void OnEnter(Entity& entity);
	virtual void OnExit(Entity& entity);
	virtual void OnUpdate(Entity& entity);
};

//using EntityStateID = CatTypeID<EntityState>;

