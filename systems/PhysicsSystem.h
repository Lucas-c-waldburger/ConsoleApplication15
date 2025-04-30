#pragma once
#include "../physics/B2World.h"
#include "../scripting/ScriptManager.h"
#include "../components/util/EventObserverUtils.h"
#include "../events/custom/CustomEventDataRegistry.h"

class PhysicsSystem
{
public:
	void Update(B2World* world_, float timeStep, int subStepCount);

private:
};

