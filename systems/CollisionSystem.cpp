#include "CollisionSystem.h"
#include "../ecs/Ecs.h"
#include "../events/custom/CustomEventDataRegistry.h"
#include "../components/util/EventObserverUtils.h"
#include "../physics/B2World.h"
#include "../core/Algorithms.h"

namespace {

template <typename T> 
concept SomeB2ContactEvent = std::same_as<T, b2ContactBeginTouchEvent> ||
							 std::same_as<T, b2ContactEndTouchEvent> ||
							 std::same_as<T, b2ContactHitEvent>;
template <typename T>
concept SomeB2SensorEvent = std::same_as<T, b2SensorBeginTouchEvent> ||
							std::same_as<T, b2SensorEndTouchEvent>;

template <typename T>
concept SomeB2Event = SomeB2ContactEvent<T> || SomeB2SensorEvent<T>;

template <typename T>
concept SomeEntityCollisionEvent = std::same_as<T, EntityCollision::ContactBegin> ||
								   std::same_as<T, EntityCollision::ContactEnd> ||
								   std::same_as<T, EntityCollision::SensorBegin> ||
								   std::same_as<T, EntityCollision::SensorEnd> ||
								   std::same_as<T, EntityCollision::Hit>;

template <SomeB2Event T>
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

template <SomeEntityCollisionEvent T, SomeB2Event U>
std::vector<T> AssembleCollisionEvents(std::vector<Entity>& entities, U* b2EvArr, int count)
{
	std::vector<T> convertedEvents;
	convertedEvents.reserve(count);

	for (int i = 0; i < count; ++i)
	{
		U* b2Ev = b2EvArr + i;

		auto [shapeHandleA, shapeHandleB] = GetShapeHandles(b2Ev);

		// shape may have been destroyed after event gathered
		if (!(shapeHandleA.IsValid() && shapeHandleB.IsValid()))
		{
			continue;
		}

		convertedEvents.push_back(T{
			.a = {.shapeHandle = shapeHandleA },
			.b = {.shapeHandle = shapeHandleB }
		});
	}

	// match shapeIds to owning collider entities
	for (auto& event : convertedEvents)
	{
		for (auto& entity : entities)
		{
			// previous rounds of event dispatching may have invalidated this entity
			if (!entity.IsValid() || !entity.HasComponent<Collider>())
			{
				continue;
			}

			auto& collider = entity.GetComponent<Collider>();
			auto& [a, b] = event;

			if (a.shapeHandle == collider.shape.GetData().GetHandle())
			{
				a.entityId = entity.GetID();
			}
			if (b.shapeHandle == collider.shape.GetData().GetHandle())
			{
				b.entityId = entity.GetID();
			}
		}
	}

	EraseIf(convertedEvents, [](const auto& ev) {
		return ev.a.entityId == kInvalidEntity || 
			   ev.b.entityId == kInvalidEntity;
	});

	return convertedEvents;
}

template <SomeEntityCollisionEvent T, SomeB2Event U>
Result<Void> SendCollisionEvents(std::vector<Entity>& entities, U* b2EvArr, int count)
{
	if (count > 0)
	{
		auto collisionEvents = AssembleCollisionEvents<T>(entities, b2EvArr, count);

		for (auto&& ev : collisionEvents)
		{
			TRY(SendEventNotification(std::move(ev)));
		}
	}

	return Void{};
}

} // unnamed namespace

Result<Void> DispatchCollisionEvents(const B2World* world)
{
	assert(world);
	assert(world->IsValid());

	auto entities = ECS::GetAllEntitiesWith<Collider>([](const Collider& collider) {
		return collider.shape.GetData().IsValid();
	});

	auto contactEvs = b2World_GetContactEvents(world->GetID());
	auto sensorEvs = b2World_GetSensorEvents(world->GetID());

	TRY(SendCollisionEvents<EntityCollision::ContactEnd>(
		entities, contactEvs.endEvents, contactEvs.endCount
	));

	TRY(SendCollisionEvents<EntityCollision::SensorEnd>(
		entities, sensorEvs.endEvents, sensorEvs.endCount
	));

	TRY(SendCollisionEvents<EntityCollision::ContactBegin>(
		entities, contactEvs.beginEvents, contactEvs.beginCount
	));

	TRY(SendCollisionEvents<EntityCollision::SensorBegin>(
		entities, sensorEvs.beginEvents, sensorEvs.beginCount
	));

	TRY(SendCollisionEvents<EntityCollision::Hit>(
		entities, contactEvs.hitEvents, contactEvs.hitCount
	));
	
	return Void{};
}
