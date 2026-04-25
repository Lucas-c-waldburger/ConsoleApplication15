#include <ranges>
#include "CollisionSystem.h"
#include "../ecs/Ecs.h"
#include "../physics/B2World.h"
#include "../core/Algorithms.h"
#include "../events/data/EntityCollision.h"

namespace {

template <typename T> 
concept SomeB2ContactEvent = std::same_as<T, b2ContactBeginTouchEvent> ||
							 std::same_as<T, b2ContactEndTouchEvent> ||
							 std::same_as<T, b2ContactHitEvent>;
template <typename T>
concept SomeB2SensorEvent = std::same_as<T, b2SensorBeginTouchEvent> ||
							std::same_as<T, b2SensorEndTouchEvent>;

template <typename T>
concept SomeB2CollisionEvent = SomeB2ContactEvent<T> || SomeB2SensorEvent<T>;

template <typename T>
concept SomeCustomCollisionEvent = std::same_as<T, events::ContactCollisionBegin> ||
								   std::same_as<T, events::ContactCollisionEnd> ||
								   std::same_as<T, events::SensorCollisionBegin> ||
								   std::same_as<T, events::SensorCollisionEnd> ||
								   std::same_as<T, events::HitCollision>;


template <SomeB2CollisionEvent T>
std::pair<Handle<B2Shape>, Handle<B2Shape>> GetShapeHandles(T* b2Ev)
{
	if constexpr (SomeB2ContactEvent<T>)
	{
		return std::make_pair(Handle<B2Shape>::Create(b2Ev->shapeIdA),
							  Handle<B2Shape>::Create(b2Ev->shapeIdB));
	}
	else // Sensor
	{
		return std::make_pair(Handle<B2Shape>::Create(b2Ev->sensorShapeId),
							  Handle<B2Shape>::Create(b2Ev->visitorShapeId));
	}
}

//template <SomeEntityCollisionEvent T, SomeB2Event U>
//std::vector<T> AssembleCollisionEvents(std::vector<Entity>& entities, U* b2EvArr, int count)
//{
//	std::vector<T> convertedEvents;
//	convertedEvents.reserve(count);
//
//	for (int i = 0; i < count; ++i)
//	{
//		U* b2Ev = b2EvArr + i;
//
//		auto [shapeHandleA, shapeHandleB] = GetShapeHandles(b2Ev);
//
//		// shape may have been destroyed after event gathered
//		if (!(shapeHandleA.IsValid() && shapeHandleB.IsValid()))
//		{
//			continue;
//		}
//
//		convertedEvents.push_back(T{
//			.a = {.shapeHandle = shapeHandleA },
//			.b = {.shapeHandle = shapeHandleB }
//		});
//	}
//
//	// match shapeIds to owning collider entities
//	for (auto& event : convertedEvents)
//	{
//		for (auto& entity : entities)
//		{
//			// previous rounds of event dispatching may have invalidated this entity
//			if (!entity.IsValid() || !entity.HasComponent<Collider>())
//			{
//				continue;
//			}
//
//			auto& collider = entity.GetComponent<Collider>();
//			auto& [a, b] = event;
//
//			if (a.shapeHandle == collider.shape.GetData().GetHandle())
//			{
//				a.entityId = entity.GetID();
//			}
//			if (b.shapeHandle == collider.shape.GetData().GetHandle())
//			{
//				b.entityId = entity.GetID();
//			}
//		}
//	}
//
//	EraseIf(convertedEvents, [](const auto& ev) {
//		return ev.a.entityId == kInvalidEntity || 
//			   ev.b.entityId == kInvalidEntity;
//	});
//
//	return convertedEvents;
//}
//
//template <SomeEntityCollisionEvent T, SomeB2Event U>
//Result<Void> SendCollisionEvents(std::vector<Entity>& entities, U* b2EvArr, int count)
//{
//	if (count > 0)
//	{
//		auto collisionEvents = AssembleCollisionEvents<T>(entities, b2EvArr, count);
//
//		for (auto&& ev : collisionEvents)
//		{
//			TRY(SendEventNotification(std::move(ev)));
//		}
//	}
//
//	return Void{};
//}

Entity FindShapeEntity(const std::vector<Entity>& entities, const Handle<B2Shape>& handle)
{
	auto it = core::FindIf(entities, [&handle](const auto& entity) {
		assert(entity.HasComponent<Collider>());
		return entity.GetComponent<Collider>().shape.GetData().GetHandle() == handle;
	});

	return (it != entities.end()) ? *it : Entity{};
};

Entity_t FindOwningBodyEntity(Entity& shapeEnt, const Handle<B2Shape>& handle)
{
	assert(shapeEnt.IsValid());
	assert(shapeEnt.HasComponent<Collider>());

	if (shapeEnt.HasComponent<RigidBody>())
	{
		assert(shapeEnt.GetComponent<RigidBody>().body.GetData().OwnsShape(handle));

		return shapeEnt.GetID();
	}

	auto& colliderShape = shapeEnt.GetComponent<Collider>().shape;
	auto parentBodyHandle = colliderShape.GetData().GetParentBodyHandle();

	auto rels = shapeEnt.GetRelations();
	assert(rels.IsChild());

	auto parent = rels.GetParent();

	assert(parent.IsValid());
	assert(parent.HasComponent<RigidBody>());
	assert(parent.GetComponent<RigidBody>().body.GetData().GetHandle() == 
		   parentBodyHandle);

	return parent.GetID();
}

template <SomeCustomCollisionEvent T, SomeB2CollisionEvent U>
void BufferCollisionEventsImpl(std::vector<Entity>& entities, EventBus& bus, 
							   U* b2EventArray, int count)
{
	for (int i = 0; i < count; ++i)
	{
		U* b2Ev = b2EventArray + i;

		auto [shapeHandleA, shapeHandleB] = GetShapeHandles(b2Ev);

		if (!(shapeHandleA.IsValid() && shapeHandleB.IsValid()))
		{
			continue;
		}

		auto entityA = FindShapeEntity(entities, shapeHandleA);
		auto entityB = FindShapeEntity(entities, shapeHandleB);

		if (!(entityA.IsValid() && entityB.IsValid()))
		{
			continue;
		}

		auto owningBodyA = FindOwningBodyEntity(entityA, shapeHandleA);
		auto owningBodyB = FindOwningBodyEntity(entityB, shapeHandleB);

		T ev{
			.a = { .entity = entityA.GetID(), .shapeHandle = shapeHandleA },
			.b = { .entity = entityB.GetID(), .shapeHandle = shapeHandleB }
		};

		ev.entity<0>() = owningBodyA;
		ev.entity<1>() = owningBodyB;

		bus.PushEvent(std::move(ev));
	}
}

} // unnamed namespace

Result<Void> DispatchCollisionEvents(const B2World* world, EventBus& bus)
{
	assert(world);
	assert(world->IsValid());

	auto entities = ECS::GetAllEntitiesWith<Collider>() |
		std::views::filter([](const auto& e) {
			return e.GetComponent<Collider>().shape.GetData().IsValid();
	}) | std::ranges::to<std::vector>();

	if (entities.empty())
	{
		return Void{};
	}

	auto contactEvs = b2World_GetContactEvents(world->GetID());
	auto sensorEvs = b2World_GetSensorEvents(world->GetID());

	BufferCollisionEventsImpl<events::ContactCollisionEnd>(
		entities, bus, contactEvs.endEvents, contactEvs.endCount
	);
	BufferCollisionEventsImpl<events::SensorCollisionEnd>(
		entities, bus, sensorEvs.endEvents, sensorEvs.endCount
	);
	BufferCollisionEventsImpl<events::ContactCollisionBegin>(
		entities, bus, contactEvs.beginEvents, contactEvs.beginCount
	);
	BufferCollisionEventsImpl<events::SensorCollisionBegin>(
		entities, bus, sensorEvs.beginEvents, sensorEvs.beginCount
	); 
	BufferCollisionEventsImpl<events::HitCollision>(
		entities, bus, contactEvs.hitEvents, contactEvs.hitCount
	);

	return Void{};
}
