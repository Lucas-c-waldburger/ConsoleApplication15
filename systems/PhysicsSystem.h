#pragma once
#include "../physics/B2World.h"
#include "System.h"
#include "../scripting/ScriptManager.h"
#include "../components/util/EventObserverUtils.h"
#include "../events/custom/CustomEventDataRegistry.h",
#include "../core/ReadOnly.h"

class PhysicsSystem : public System, public HasWriteAccess<PhysicsSystem, B2Body>
{
public:
	void Update(B2World* world_, float timeStep, int subStepCount);

private:
	void UpdateForces();
};

