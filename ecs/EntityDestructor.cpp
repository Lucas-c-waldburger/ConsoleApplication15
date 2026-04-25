#include "EntityDestructor.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include "EntityRelationsHelper.h"
//#include "../components/RigidBodyComponent.h"
//#include "../components/ColliderComponent.h"
#include "../core/ReadOnly.h"
#include "../events/data/EntityActions.h"
#include "../physics/B2Body.h";
#include "../events/EventBus2.h"

namespace {

WriteAccessor<B2Body> bodyAccessor;
WriteAccessor<B2Shape> shapeAccessor;

void CleanupB2Components(ComponentManager& componentManager, Entity_t entity)
{
	const bool hasCollider = componentManager.HasComponent<Collider>(entity);
	const bool hasBody = componentManager.HasComponent<RigidBody>(entity);

	if (hasBody)
	{
		auto& rigidBody = componentManager.GetComponent<RigidBody>(entity);
		auto& body = bodyAccessor(rigidBody.body);

		if (body.IsValid())
		{
			body.Destroy();
		}
	}
	else if (hasCollider)
	{
		auto& collider = componentManager.GetComponent<Collider>(entity);
		auto& shape = shapeAccessor(collider.shape);

		if (shape.IsValid())
		{
			shape.Destroy();
		}		
	}
}

std::vector<events::EntityDestroyed> GetAllEntitiesToDestroy(EntityManager& entityManager, 
															 ComponentManager& componentManager, 
															 Entity_t entityId)
{
	std::vector<events::EntityDestroyed> entitiesToDestroy{{ .entity = entityId }}; 

	bool isParent = EntityRelationsHelper::IsParent(entityManager, componentManager, entityId);
	bool isChild = EntityRelationsHelper::IsChild(entityManager, componentManager, entityId);

	if (!(isParent || isChild))
	{
		return entitiesToDestroy;
	}

	Entity_t root = (isChild) ? 
		EntityRelationsHelper::GetParent(entityManager, componentManager, entityId) : entityId;
	auto& children = EntityRelationsHelper::GetChildren(entityManager, componentManager, root);

	if (isParent)
	{
		entitiesToDestroy.reserve(children.size() + 1);

		for (auto child : children)
		{
			entitiesToDestroy.push_back({
				.entity = child,
				.parent = entityId
			});
		}
	}
	else // is child
	{
		entitiesToDestroy.front().parent = root;

		assert(children.contains(entityId));

		children.erase(entityId);
	}
	
	return entitiesToDestroy;
}

} // unnamed


void EntityDestructor::EntityDestroyed(EntityManager& entityManager, ComponentManager& componentManager,
									   /*EventBus2& bus, */Entity_t entityId)
{
	auto entitiesToDestroy = GetAllEntitiesToDestroy(entityManager, componentManager, entityId);

	for (auto&& destructionEvent : entitiesToDestroy)
	{
		CleanupB2Components(componentManager, destructionEvent.entity);

		assert(componentManager.HasComponent<EntityFlags>(destructionEvent.entity));
		const auto& flags = componentManager.GetComponent<EntityFlags>(destructionEvent.entity);

		entityManager.DestroyEntity(destructionEvent.entity);

		//if (flags.eventProductionFlags.Test<events::EntityDestroyed>())
		//{
			//	bus.PushEvent(std::move(destructionEvent));
		//}


		componentManager.EntityDestroyed(destructionEvent.entity);
	}

	//bus.DispatchEvents();
}
