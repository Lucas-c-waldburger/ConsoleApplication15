#pragma once
#include "../physics/B2Joint.h"
#include "../physics/B2World.h"
#include "../ecs/Ecs.h"
#include "../components/builder/RigidBodyComponentBuilder.h"
#include "../core/commonObjects.h"
#include "../core/Algorithms.h"

namespace test
{
	/*template <ComponentType T>
	std::vector<T*> ExtractComponent(const std::vector<Entity>& entities)
	{
		std::vector<T*> extracted{};
		extracted.reserve(entities.size());
		
		for (const auto& entity : entities)
		{
			if (entity.HasComponent<T>())
			{
				extracted.push_back(&entity.GetComponent<T>());
			}
		}

		extracted.shrink_to_fit();

		return extracted;
	}

	template <ComponentType...Ts>
	std::vector<std::tuple<Ts*...>> ExtractComponents(const std::vector<Entity>& entities)
	{
		std::vector<std::tuple<Ts*...>> extracted;
		extracted.reserve(entities.size());

		auto extract = []<typename T, size_t I>(const auto& entity, auto& tup) {
			if (entity.HasComponent<T>())
			{
				std::get<I>(tup) = &entity.GetComponent<T>();
			}
			else
			{
				std:get<I>(tup) = nullptr;
			}
		};

		for (const auto& entity : entities)
		{
			auto& newTup = extracted.emplace_back();

			[&entity, &newTup]<size_t...Is>(std::index_sequence) {
				((extract<Ts, Is>(entity, newTup)), ...);
			}(std::make_index_sequence<sizeof...(Ts)>{});
		}

		extracted.shrink_to_fit();

		return extracted;
	}*/


	inline std::vector<Collider*> FindCollidersForEntity(Entity& entity)
	{
		if (!entity.HasComponent<RigidBody>())
		{
			return {};
		}

		std::vector<Collider*> colliders{};

		auto& rigidBody = entity.GetComponent<RigidBody>();

		auto shapes = rigidBody.body.GetData().GetShapes();

		auto shapeEntities = ECS::GetAllEntitiesWith<Collider>([&shapes](const Collider& collider) {
			return collider.shape.GetData().IsValid() &&
				std::any_of(shapes.begin(), shapes.end(), 
					[target = collider.shape](const auto& shape) { return target == shape; });
		});

		colliders.reserve(shapeEntities.size() + 1);

		std::transform(shapeEntities.begin(), shapeEntities.end(), std::back_inserter(colliders),
			[](auto& ent) { return &ent.GetComponent<Collider>(); });

		return colliders;
	}

	inline std::vector<Entity> FindColliderEntitiesForRigidBody(const RigidBody& rigidBody)
	{
		if (!rigidBody.body.GetData().IsValid())
		{
			return {};
		}

		auto shapes = rigidBody.body.GetData().GetShapes();

		auto shapeEntities = ECS::GetAllEntitiesWith<Collider>([&shapes](const Collider& collider) {
			return collider.shape.GetData().IsValid() &&
				std::any_of(shapes.begin(), shapes.end(),
					[target = collider.shape](const auto& shape) { return target == shape; });
			});

		return shapeEntities;
	}

	inline std::optional<Entity> FindRigidBodyEntityForCollider(const Collider& collider)
	{
		const auto& shape = collider.shape.GetData();

		if (!shape.IsValid())
		{
			return std::nullopt;
		}

		auto entities = ECS::GetAllEntitiesWith<RigidBody>(
			[bodyHandle = shape.GetParentBodyHandle()](const RigidBody& rigidBody) {
				return bodyHandle == rigidBody.body.GetData().GetHandle();
			});

		if (entities.empty())
		{
			return std::nullopt;
		}

		assert(entities.size() <= 1);

		return entities.back();
	}

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

		auto& parentEntityId = entity.GetComponent<Parent>().entityId;
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

	
	
	



	/*inline void HashCombine(std::size_t& seed, std::size_t value)
	{
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}*/




}

//#include "../inputs/InputState.h"
//#include <vector>
//
//class GameControllerInput
//{
//public:
//	enum Input
//	{
//		Invalid = SDL_CONTROLLER_BUTTON_INVALID,
//		A = SDL_CONTROLLER_BUTTON_A,
//		B = SDL_CONTROLLER_BUTTON_B,
//		X = SDL_CONTROLLER_BUTTON_X,
//		Y = SDL_CONTROLLER_BUTTON_Y,
//		Back = SDL_CONTROLLER_BUTTON_BACK,
//		Guide = SDL_CONTROLLER_BUTTON_GUIDE,
//		Start = SDL_CONTROLLER_BUTTON_START,
//		LeftStickButton = SDL_CONTROLLER_BUTTON_LEFTSTICK,
//		RightStickButton = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
//		LeftShoulder = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
//		RightShoulder = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
//		DPadUp = SDL_CONTROLLER_BUTTON_DPAD_UP,
//		DPadDown = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
//		DPadLeft = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
//		DPadRight = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
//		Misc1 = SDL_CONTROLLER_BUTTON_MISC1,
//		Paddle1 = SDL_CONTROLLER_BUTTON_PADDLE1,
//		Paddle2 = SDL_CONTROLLER_BUTTON_PADDLE2,
//		Paddle3 = SDL_CONTROLLER_BUTTON_PADDLE3,
//		Paddle4 = SDL_CONTROLLER_BUTTON_PADDLE4,
//		TouchPad = SDL_CONTROLLER_BUTTON_TOUCHPAD,
//		LeftStickAxis = SDL_CONTROLLER_BUTTON_MAX,
//		RightStickAxis,
//		LeftTrigger,
//		RightTrigger
//	};
//
//private:
//	static constexpr int kButtonEnumStart = Input::A;
//	static constexpr int kButtonEnumEnd = Input::TouchPad;
//	static constexpr int kAxisEnumStart = Input::LeftStickAxis;
//	static constexpr int kAxisEnumEnd = Input::RightTrigger;
//
//public:
//	static constexpr size_t kInputsSize = kAxisEnumEnd - kButtonEnumStart;
//
//	static constexpr bool IsButton(Input inp) noexcept {
//		return inp >= kButtonEnumStart && inp <= kButtonEnumEnd;
//	}
//	static constexpr bool IsAxis(Input inp) noexcept {
//		return inp >= kAxisEnumStart && inp <= kAxisEnumEnd;
//	}
//};

//template <typename Context>
//struct SchemeInputDefinition
//{
//	GameControllerInput input;
//	InputState state;
//	std::function<void(Context&)> callback;
//
//	bool operator==(const SchemeInputDefinition& rhs) const { 
//		return input == rhs.input && state == rhs.state; 
//	}
//};
//
//template <typename Context>
//class GameControllerScheme
//{
//public:
//	GameControllerScheme() = default;
//
//	bool OnInput(GameControllerInput inp, InputState state, std::function<void(Context&)> fn)
//	{
//		if (!fn)
//		{
//			return false;
//		}
//		if (!(GameControllerInput::IsButton(inp) || GameControllerInput::IsAxis(inp)))
//		{
//			return false;
//		}
//
//		inputDefs_.push_back(SchemeInputDefinition<Context>{
//			.input = inp,
//			.state = state,
//			.callback = std::move(fn)
//		});
//	}
//
//	void Process(const GameControllerState& gc)
//	{
//		auto getAxis = [&gc](auto inp) -> AxisInputData* {
//			return (inp == GameControllerInput::LeftStickAxis) ? &gc.axisInput.left :
//			       (inp == GameControllerInput::RightStickAxis) ? &gc.axisInput.right : nullptr;
//		};
//
//		for (auto& [input, state, callback] : inputDefs_)
//		{
//			if (!callback)
//			{
//				continue;
//			}
//
//			if (GameControllerInput::IsButton(input))
//			{
//				auto btnState = gc.buttonInput[static_cast<size_t>(input)].state;
//				
//				if (btnState == state)
//				{
//					callback(context_);
//				}
//			}
//			else if (GameControllerInput::IsAxis(input))
//			{			
//				auto axisPtr = getAxis(input);
//				if (!axisPtr)
//				{
//					continue;
//				}
//
//				if (axisState == state)
//				{
//					callback(context_);
//				}				
//			}
//		}
//	}
//
//private:
//	Context context_;
//	std::vector<SchemeInputDefinition<Context>> inputDefs_;
//};
//
//namespace std {
//	template <typename Context>
//	struct hash<SchemeInputDefinition<Context>> {
//		size_t operator()(const SchemeInputDefinition<Context>& def) const noexcept {
//			std::size_t hash = 0;
//			test::HashCombine(hash, std::hash<int>{}(static_cast<int>(def.input));
//			test::HashCombine(hash, std::hash<int>{}(static_cast<int>(def.state));
//
//			return hash;
//		}
//	};
//}