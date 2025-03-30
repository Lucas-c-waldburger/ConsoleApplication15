#include "PhysicsSystem.h"
#include "../ecs/Ecs.h"
#include "util/QuadTree.h"
#include <algorithm>

namespace {

SDL_FPoint ResolveAccumulatedForces(AccumulatedForces& forces, float deltaTime)
{
	SDL_FPoint netForce{ 0.0f, 0.0f };

	for (auto it = forces.normed.begin(); it != forces.normed.end();)
	{
		netForce.x += it->vector.x;
		netForce.y += it->vector.y;

		it->duration -= deltaTime;
		if (it->duration <= 0)
		{
			it = forces.normed.erase(it);
		}
		else
		{
			it++;
		}
	}

	netForce.x *= forces.max;
	netForce.y *= forces.max;

	return netForce;
}

// TODO : take force duration into account
void ApplyForces(Physics& physics, float deltaTime) // UpdateAcceleration?
{
	SDL_FPoint netForce = ResolveAccumulatedForces(physics.forces, deltaTime);

	physics.acceleration.y += physics.gravity;

	assert(physics.mass >= 0.01f);

	physics.acceleration.x += netForce.x / physics.mass;
	physics.acceleration.y += netForce.y / physics.mass;
}

void ApplyAcceleration(Physics& physics, float deltaTime)
{
	physics.velocity.x += physics.acceleration.x * deltaTime;
	physics.velocity.y += physics.acceleration.y * deltaTime;
}

void ApplyDrag(Physics& physics, float deltaTime)
{
	physics.velocity.x *= 1.0f - (physics.drag / physics.mass * deltaTime);
	physics.velocity.y *= 1.0f - (physics.drag / physics.mass * deltaTime);
}

void UpdatePosition(Spatial& spatial, Physics& physics, float deltaTime)
{
	spatial.position.x += physics.velocity.x * deltaTime;
	spatial.position.y += physics.velocity.y * deltaTime;
}

SDL_FPoint PredictPosition(Spatial& spatial, Physics& physics, float deltaTime)
{
	return { spatial.position.x + (physics.velocity.x * deltaTime),
			 spatial.position.y + (physics.velocity.y * deltaTime) };
}

} // unnamed namespace

void PhysicsSystem::Update(float deltaTime)
{
	auto entities = ECS::GetAllEntitiesWith<Physics, Spatial>();

	for (auto& entity : entities)
	{
		auto& physics = entity.GetComponent<Physics>();
		auto& spatial = entity.GetComponent<Spatial>();

		ApplyForces(physics, deltaTime);
		ApplyAcceleration(physics, deltaTime);
		ApplyDrag(physics, deltaTime);

		UpdatePosition(spatial, physics, deltaTime);


	}
}

void PhysicsSystem::RunEntityScripts(ScriptManager& scriptManager)
{
	auto entities = ECS::GetAllEntitiesWith<Script, Physics>(
		[](const Script& script, const Physics&) 
		{
			return !script.activeScript.name.empty() &&
				   (script.systemDomain == typeid(PhysicsSystem));
		});

	for (auto& entity : entities)
	{
		auto& script = entity.GetComponent<Script>();

		LOG_IF_ERROR(scriptManager.RunScript(script.activeScript.name));
	}
}