#pragma once
#include "System.h"
#include "../core/ReadOnly.h"
#include "../physics/B2Body.h"
#include "../events/EventBus2.h"

class B2World;

class PhysicsSystem : public System, 
					  public HasWriteAccess<PhysicsSystem, B2Body>
{
public:
	void Update(B2World* world_, EventBus& bus, float timeStep, int subStepCount);

private:
	void UpdateForces();
};

