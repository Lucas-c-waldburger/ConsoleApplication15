#include "PhysicsSystem.h"
#include "../ecs/Ecs.h"
#include "../sdl/SDLUtils.h"
#include "CollisionSystem.h"
#include <algorithm>

namespace {

float GetLengthSquared(const SDL_FPoint& point)
{
	return point.x * point.x + point.y * point.y;
}

void ClampVelocities(B2Body& body, const BodyLimits& limits)
{
	assert(body.IsValid());

	SDL_FPoint linearVel = body.GetLinearVelocity();
	SDL_FPoint maxLinear = limits.linearVelocity.max;
	if (std::abs(linearVel.x) > maxLinear.x || std::abs(linearVel.y) > maxLinear.y)
	{
		linearVel.x = std::clamp(linearVel.x, -maxLinear.x, maxLinear.x);
		linearVel.y = std::clamp(linearVel.y, -maxLinear.y, maxLinear.y);

		body.SetLinearVelocity(linearVel);
	}

	float angularVel = body.GetAngularVelocity();
	float maxAngular = limits.angularVelocity.max;
	if (std::abs(angularVel) > maxAngular)
	{
		angularVel = std::clamp(angularVel, -maxAngular, maxAngular);

		body.SetAngularVelocity(angularVel);
	}
}

void ApplyForceRequestsImpl(B2Body& body, std::vector<Force>& forces, 
							void (B2Body::*applyFn)(SDL_FPoint, std::optional<SDL_FPoint>))
{
	if (forces.empty())
	{
		return;
	}

	SDL_FPoint totalCenterForce = { 0.0f, 0.0f };
	for (const auto& [value, worldPoint] : forces)
	{
		if (!worldPoint.has_value())
		{
			totalCenterForce += value;
		}
		else
		{
			(body.*applyFn)(value, worldPoint);
		}
	}

	if (GetLengthSquared(totalCenterForce) > 0.0f)
	{
		(body.*applyFn)(totalCenterForce, std::nullopt);
	}

	forces.clear();
}

void ApplyForceRequests(B2Body& body, ForceRequests& requests)
{
	ApplyForceRequestsImpl(body, requests.forces, &B2Body::ApplyForce);
	ApplyForceRequestsImpl(body, requests.impulses, &B2Body::ApplyLinearImpulse);
}

void UpdateForces(const B2World* world)
{
	assert(world);

	auto entities = ECS::GetAllEntitiesWith<RigidBody>([](const RigidBody& rigidBody) {
		return rigidBody.bodyHandle.IsValid();
	});

	for (auto& entity : entities)
	{
		auto& rigidBody = entity.GetComponent<RigidBody>();

		auto bodyOc = world->GetBody(rigidBody.bodyHandle);
		if (!bodyOc.Success())
		{
			LOG_ERROR(bodyOc.GetError());
			continue;
		}

		auto& body = bodyOc.GetValue();

		ApplyForceRequests(body, rigidBody.forceRequests);

		ClampVelocities(body, rigidBody.limits);
	}
}

void UpdateTransformComponents(const B2World* world)
{
	assert(world);

	auto entities = ECS::GetAllEntitiesWith<RigidBody, Transform>(
		[](const RigidBody& rigidBody, const Transform&) {
			return rigidBody.bodyHandle.IsValid();
	});

	for (auto& entity : entities)
	{
		auto& rigidBody = entity.GetComponent<RigidBody>();
		auto& transform = entity.GetComponent<Transform>();

		auto bodyOc = world->GetBody(rigidBody.bodyHandle);
		if (!bodyOc.Success())
		{
			LOG_ERROR(bodyOc.GetError());
			continue;
		}

		auto& body = bodyOc.GetValue();

		transform.position = body.GetPosition();
		transform.rotation = body.GetAngle();
	}
}

} // unnamed namespace

void PhysicsSystem::Update(B2World* world_, float timeStep, int subStepCount)
{
	if (!world_)
	{
		LOG_ERROR("B2World was null");
		return;
	}
	if (!world_->IsValid())
	{
		LOG_ERROR("B2World was invalid");
		return;
	}

	UpdateForces(world_);

	world_->Step(timeStep, subStepCount);

	LOG_IF_ERROR(DispatchCollisionEvents(world_));

	UpdateTransformComponents(world_);
}






//namespace {
//
//SDL_FPoint ResolveAccumulatedForces(AccumulatedForces& forces, float deltaTime)
//{
//	SDL_FPoint netForce{ 0.0f, 0.0f };
//
//	for (auto it = forces.normed.begin(); it != forces.normed.end();)
//	{
//		netForce.x += it->vector.x;
//		netForce.y += it->vector.y;
//		//netForce.x += it->vector.x * (std::min(it->duration, deltaTime) / deltaTime);
//		//netForce.y += it->vector.y * (std::min(it->duration, deltaTime) / deltaTime);
//
//		it->duration -= deltaTime;
//		if (it->duration <= 0)
//		{
//			it = forces.normed.erase(it);
//		}
//		else
//		{
//			it++;
//		}
//	}
//
//	return netForce;
//}
//
//// TODO : take force duration into account
//void ApplyForces(Physics& physics, float deltaTime) // UpdateAcceleration?
//{
//	SDL_FPoint netForce = ResolveAccumulatedForces(physics.forces, deltaTime);
//
//	netForce.y += (physics.gravity * physics.forces.max);
//
//	netForce.x *= physics.forces.max;
//	netForce.y *= physics.forces.max;
//
//	assert(physics.mass >= 0.01f);
//
//	physics.acceleration.x = netForce.x / physics.mass;
//	physics.acceleration.y = netForce.y / physics.mass;// + physics.gravity;
//
//	//physics.acceleration.x = (netForce.x / physics.mass) * deltaTime;
//	//physics.acceleration.y = (netForce.y / physics.mass) * deltaTime;
//
//	//physics.acceleration.y += physics.gravity;
//}
//
//void ApplyAcceleration(Physics& physics, float deltaTime)
//{
//	physics.velocity.x += physics.acceleration.x * deltaTime;
//	physics.velocity.y += physics.acceleration.y * deltaTime;
//}
//
//void ApplyDrag(Physics& physics, float deltaTime)
//{
//	float dragFactor = 1.0f - (physics.drag * deltaTime);
//	physics.velocity.x *= dragFactor;
//	physics.velocity.y *= dragFactor;
//
//	//float dragCoefficient = physics.drag / physics.mass;
//	//physics.velocity.x *= 1.0f - (dragCoefficient * deltaTime);
//	//physics.velocity.y *= 1.0f - (dragCoefficient * deltaTime);
//
//	//physics.velocity.x *= 1.0f - (physics.drag / physics.mass * deltaTime);
//	//physics.velocity.y *= 1.0f - (physics.drag / physics.mass * deltaTime);
//	 
//	//physics.velocity.x *= physics.drag;
//	//physics.velocity.y *= physics.drag;
//}
//
//void UpdatePosition(Spatial& spatial, Physics& physics, float deltaTime)
//{
//	spatial.position.x += physics.velocity.x * deltaTime;
//	spatial.position.y += physics.velocity.y * deltaTime;
//}
//
//SDL_FPoint ProjectPosition(Spatial& spatial, Physics& physics, float deltaTime)
//{
//	return { spatial.position.x + (physics.velocity.x * deltaTime),
//			 spatial.position.y + (physics.velocity.y * deltaTime) };
//}
//
//void UpdateCollider(SDL_FPoint prevPos, SDL_FPoint projPos, Collider& collider)
//{
//	float dx = projPos.x - prevPos.x;
//	float dy = projPos.y - prevPos.y;
//
//	collider.position.x += dx;
//	collider.position.y += dy;
//}
//
//} // unnamed namespace
//
//void PhysicsSystem::Update(CollisionSystem& collisionSystem, float deltaTime)
//{
//	auto physEntities = ECS::GetAllEntitiesWith<Physics, Spatial>();
//
//	std::unordered_map<Entity_t, SDL_FPoint> colliderPosCache;
//
//	for (auto& entity : physEntities)
//	{
//		auto& physics = entity.GetComponent<Physics>();
//		auto& spatial = entity.GetComponent<Spatial>();
//
//		ApplyForces(physics, deltaTime);
//		ApplyAcceleration(physics, deltaTime);
//		ApplyDrag(physics, deltaTime);
//
//		SDL_FPoint projectedPos = ProjectPosition(spatial, physics, deltaTime);
//
//		if (entity.HasComponent<Collider>())
//		{
//			auto& collider = entity.GetComponent<Collider>();
//
//			UpdateCollider(spatial.position, projectedPos, collider);
//
//			colliderPosCache.emplace(entity.GetID(), collider.position);
//		}
//
//		spatial.position = projectedPos;
//	}
//
//	collisionSystem.HandleCollisions();
//
//	// update spatial position based on collider delta
//	auto colEntities = ECS::GetAllEntitiesWith<Collider, Spatial>();
//
//	for (auto& entity : colEntities)
//	{
//		auto& collider = entity.GetComponent<Collider>();
//		auto& spatial = entity.GetComponent<Spatial>();
//
//		auto it = colliderPosCache.find(entity.GetID());
//		if (it == colliderPosCache.end())
//		{
//			continue;
//		}
//
//		SDL_FPoint oldColliderPos = it->second;
//
//		float dx = collider.position.x - oldColliderPos.x;
//		float dy = collider.position.y - oldColliderPos.y;
//
//		spatial.position.x += dx;
//		spatial.position.y += dy;
//	}
//
//}

//void PhysicsSystem::RunEntityScripts(ScriptManager& scriptManager)
//{
//	auto entities = ECS::GetAllEntitiesWith<Script, Physics>(
//		[](const Script& script, const Physics&) 
//		{
//			return !script.activeScript.name.empty() &&
//				   (script.systemDomain == typeid(PhysicsSystem));
//		});
//
//	for (auto& entity : entities)
//	{
//		auto& script = entity.GetComponent<Script>();
//
//		LOG_IF_ERROR(scriptManager.RunScript(script.activeScript.name));
//	}
//}
