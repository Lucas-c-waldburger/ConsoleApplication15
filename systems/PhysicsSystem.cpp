#include "PhysicsSystem.h"
#include "../ecs/Ecs.h"
#include "../sdl/SDLUtils.h"
#include "../events/custom/data/Groups.h"
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

//void UpdateDependantComponents()
//{
//	auto entities = ECS::GetAllEntitiesWith<RigidBody>(
//		[](const RigidBody& rigidBody, const Transform&) {
//			return rigidBody.body.GetData().IsValid();
//	});
//
//	for (auto& entity : entities)
//	{
//		auto& rigidBody = entity.GetComponent<RigidBody>();
//		auto& transform = entity.GetComponent<Transform>();
//
//		if (entity.HasComponent<EntityMetrics>())
//		{
//			auto& metrics = entity.GetComponent<EntityMetrics>();
//			if (metrics.travelDistance.has_value())
//			{
//				auto [nowX, nowY] = rigidBody.body.GetData().GetPosition();
//				auto [lastX, lastY] = transform.position;
//
//				metrics.travelDistance->total.x += std::abs(lastX - nowX);
//				metrics.travelDistance->total.y += std::abs(lastY - nowY);
//			}
//		}
//
//		transform.position = rigidBody.body.GetData().GetPosition();
//		transform.rotation = rigidBody.body.GetData().GetAngle();
//	}
//}

void UpdateTransformComponents()
{
	auto entities = ECS::GetAllEntitiesWith<RigidBody, Transform>(
		[](const RigidBody& rigidBody, const Transform&) {
			return rigidBody.body.GetData().IsValid();
	});

	for (auto& entity : entities)
	{
		auto& rigidBody = entity.GetComponent<RigidBody>();
		auto& transform = entity.GetComponent<Transform>();

		transform.position = rigidBody.body.GetData().GetPosition();
		transform.rotation = rigidBody.body.GetData().GetAngle();
	}
}

} // unnamed namespace

void PhysicsSystem::Update(EventSystem& eventSystem, B2World* world_, float timeStep, int subStepCount)
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

	UpdateForces();

	world_->Step(timeStep, subStepCount);

	BufferCollisionEvents(world_);

	eventSystem.DispatchEvents<events::CollisionEventGroup>();

	UpdateTransformComponents();
}

void PhysicsSystem::UpdateForces()
{
	auto entities = ECS::GetAllEntitiesWith<RigidBody>([](const RigidBody& rigidBody) {
		return rigidBody.body.GetData().IsValid();
	});

	for (auto& entity : entities)
	{
		auto& rigidBody = entity.GetComponent<RigidBody>();

		auto& bodyData = GetWriteAccess(rigidBody.body);

		ApplyForceRequests(bodyData, rigidBody.forceRequests);

		ClampVelocities(bodyData, rigidBody.limits);
	}
}



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
