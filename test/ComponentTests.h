#pragma once
#include "../physics/B2Joint.h"
#include "../physics/B2World.h"
#include "../ecs/Ecs.h"
#include "../components/builder/RigidBodyComponentBuilder.h"
#include "../core/commonObjects.h"

namespace test
{
	inline Result<Entity> GetOwningBodyEntity(Entity& entity)
	{
		assert(entity.HasComponent<Collider>());

		auto& shape = entity.GetComponent<Collider>().shape;

		assert(shape.GetData().IsValid());

		auto parentBodyHandle = shape.GetData().GetParentBodyHandle();
		if (!parentBodyHandle.IsValid())
		{
			return MAKE_ERROR("associated body handle was invalid");
		}

		// check if this entity has the owning body
		if (entity.HasComponent<RigidBody>())
		{
			auto bodyHandle = entity.GetComponent<RigidBody>().body.GetData().GetHandle();

			if (bodyHandle.IsValid() && bodyHandle == parentBodyHandle)
			{
				return entity;
			}
		}

		// check if entity's parent is the owning body
		if (!entity.HasComponent<Parent>())
		{
			return MAKE_ERROR("Entity did not own collider's body and had no parent who could own it");
		}

		auto& parentEntityId = entity.GetComponent<Parent>().parentEntity;
		auto parentEntity = ECS::GetEntityByID(parentEntityId);

		if (!parentEntity.IsValid())
		{
			return MAKE_ERROR("Parent entity was invalid");
		}
		if (!parentEntity.HasComponent<RigidBody>())
		{
			return MAKE_ERROR("Parent entity did not have a rigid body component");
		}

		auto entParentBodyHandle = parentEntity.GetComponent<RigidBody>().body.GetData().GetHandle();

		if (entParentBodyHandle.IsValid() && entParentBodyHandle == parentBodyHandle)
		{
			return parentEntity;
		}

		return MAKE_ERROR("Failed to find owning body");
	}

	inline b2Vec2 ConvertIntersectionPointToLocalAnchor(const B2RayCastResult& result)
	{
		b2BodyId bodyId = b2Shape_GetBody(result.shapeHandle);

		b2Transform transform = b2Body_GetTransform(bodyId);

		return b2InvTransformPoint(transform, ToB2VecScaled(result.point));
	}

	inline Result<B2DistanceJoint> MakeGrappleJoint(B2World& world, Entity& entA, SDL_FPoint point, 
											        B2JointParams<B2DistanceJoint> jointParams)
	{
		assert(entA.IsValid());
		assert(entA.HasComponent<RigidBody>());
		
		auto& rigidBody = entA.GetComponent<RigidBody>();
	
		TRY(world.GetBody(rigidBody.body.GetData().GetHandle()), entABody);

		auto rayCastResult = world.CastRayToPoint(entABody, point);
		if (rayCastResult.hit)
		{
			// hit an existing shape
			assert(rayCastResult.shapeHandle.IsValid());

			auto shapeEntities = ECS::GetAllEntitiesWith<Collider>(
				[handle = rayCastResult.shapeHandle](const Collider& collider) {
					return collider.shape.GetData().IsValid() && 
						   handle == collider.shape.GetData().GetHandle();
				});

			if (shapeEntities.empty())
			{
				return MAKE_ERROR("Shape id found in ray cast without entity associated to it");
			}
			if (shapeEntities.size() > 1)
			{
				return MAKE_ERROR("more than one entity associated with shape id");
			}

			TRY(GetOwningBodyEntity(shapeEntities[0]), entB);

			assert(entA.GetID() != entB.GetID());
			assert(entB.HasComponent<RigidBody>());

			const auto& entBBody = entB.GetComponent<RigidBody>().body.GetData();

			assert(b2Shape_GetBody(rayCastResult.shapeHandle) == entBBody.GetHandle());

			// set local anchor on target
			//b2Transform transform = b2Body_GetTransform(entBBody.GetHandle()); 

			//b2Vec2 localAnchor = b2InvTransformPoint(transform, ToB2VecScaled(rayCastResult.point));
			b2Vec2 localAnchor = b2InvTransformPoint(
				b2Body_GetTransform(entBBody.GetHandle()), ToB2VecScaled(rayCastResult.point)
			);
			SDL_FPoint localAnchorScaled = ToSDLFPointScaled(localAnchor);

			// decide who is in which slot by y value
			const bool bodyBAboveA = entABody.GetPosition().y > entBBody.GetPosition().y;

			auto slotAHandle = (bodyBAboveA) ? entBBody.GetHandle() : entABody.GetHandle();
			auto slotBHandle = (bodyBAboveA) ? entABody.GetHandle() : entBBody.GetHandle();

			b2Vec2 worldHitPoint = ToB2VecScaled(rayCastResult.point);

			// Compute local anchor points in both bodyA and bodyB spaces
			//b2Vec2 localAnchorA = b2InvTransformPoint(b2Body_GetTransform(entABody.GetHandle()), worldHitPoint);

			// Set the anchors relative to which is assigned as A/B in the joint
			jointParams.localAnchor.a = (slotAHandle == entBBody.GetHandle()) ? localAnchorScaled : SDL_FPoint{ 0.0f, 0.0f };
			jointParams.localAnchor.b = (slotBHandle == entBBody.GetHandle()) ? localAnchorScaled : SDL_FPoint{ 0.0f, 0.0f };

			/*jointParams.localAnchor.b = (bodyBAboveA) ? ToSDLFPointScaled(localAnchor) : SDL_FPoint{ 0.0f, 0.0f };
			jointParams.localAnchor.a = (bodyBAboveA) ? SDL_FPoint{ 0.0f, 0.0f } : ToSDLFPointScaled(localAnchor);*/

			return B2JointFactory::MakeDistanceJoint(slotAHandle, slotBHandle, jointParams);
		}

		//// else we make a body here 
		//auto jointTargetEnt = ECS::CreateEntity();
		//assert(jointTargetEnt.IsValid());

		//auto& targetRigidBody = jointTargetEnt.AddComponent<RigidBody>(ComponentBuilder<RigidBody>{}
		//.WithBodyParameters({
		//	.bodyType = B2Body::Type::Static,
		//	.position = point
		//}).Build(world));

		//const auto& targetBody = targetRigidBody.body.GetData();
		//assert(targetBody.IsValid());


		//return B2JointFactory::MakeDistanceJoint(entABody.GetHandle(), targetBody.GetHandle(), jointParams);

		return B2DistanceJoint{};
	}











}