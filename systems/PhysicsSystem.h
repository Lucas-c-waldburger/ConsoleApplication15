#pragma once
#include "System.h"
#include "Observers.h"
#include "Pausable.h"
#include "../core/ReadOnly.h"
#include "../physics/B2Body.h"
#include "../physics/B2Shape.h"
#include "../events/EventBus2.h"

class B2World;

class PhysicsSystem : public System, 
					  public HasWriteAccessImpl<PhysicsSystem, B2Body, B2Shape>,
					  public Pausable<PhysicsSystem>,
					  public EntityDestroyedObserver<PhysicsSystem>,
					  public ComponentRemovedObserver<PhysicsSystem>
{
public:
	friend class Pausable<PhysicsSystem>;
	friend class EntityDestroyedObserver<PhysicsSystem>;
	friend class ComponentRemovedObserver<PhysicsSystem>;

	PhysicsSystem();

	void Update(B2World* world_, EventBus& bus, float timeStep, int subStepCount);

private:
	void UpdateForces();

	void OnEntityDestroyed(Entity e);
	void OnComponentRemoved(Entity e, ComponentSignature sig);
};

