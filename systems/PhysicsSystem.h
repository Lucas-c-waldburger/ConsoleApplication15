#pragma once
#include "../scripting/ScriptManager.h"
#include "CollisionSystem.h"
#include "../sdl/SDLite.h"
#include "../components/util/EventObserverUtils.h"
#include "../events/custom/CustomEventDataRegistry.h"

class PhysicsSystem
{
public:
	void Update(CollisionSystem& collisionSystem, float deltaTime);

private:
	void RunEntityScripts(ScriptManager& scriptManager);
};

