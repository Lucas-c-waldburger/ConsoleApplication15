#include "EntityDestructor.h"
#include "../physics/B2Body.h";
#include "EntityRelationsHelper.h"

namespace {

WriteAccessor<B2Body> bodyAccessor;
WriteAccessor<B2Shape> shapeAccessor;

//enum class Identity : uint8_t
//{
//	Independant,
//	Parent,
//	Child
//};
//
//struct EntitiesToDestroy
//{
//	std::vector<Entity_t> entityIds;
//	Identity firstEntityIdentity;
//};
//
//Identity GetEntityIdentity(impl::ComponentManager& componentManager, Entity_t entityId)
//{
//	if (componentManager.HasComponent<Parent>(entityId))
//	{
//		assert(!componentManager.HasComponent<Children>(entityId));
//
//		return Identity::Child;
//	}
//	else if (componentManager.HasComponent<Children>(entityId))
//	{
//		assert(!componentManager.HasComponent<Parent>(entityId));
//
//		return Identity::Parent;
//	}
//
//	return Identity::Independant;
//}
//
//void CleanupB2ComponentsSingleOwnership(impl::ComponentManager& componentManager, Entity_t entity)
//{
//	if (!componentManager.HasComponent<RigidBody>(entity))
//	{
//		assert(!componentManager.HasComponent<Collider>(entity));
//
//		return;
//	}
//
//	auto& rigid = componentManager.GetComponent<RigidBody>(entity);
//	auto& body = bodyAccessor(rigid.body);
//
//	if (componentManager.HasComponent<Collider>(entity))
//	{
//		auto& collider = componentManager.GetComponent<Collider>(entity);
//		auto& shape = shapeAccessor(collider.shape);
//		
//		// valid body -> body must own this shape
//		// invalid body -> must have invalid shape
//		if (!body.IsValid())
//		{
//			assert(!shape.IsValid());
//
//			return;
//		}
//
//		if (shape.IsValid())
//		{
//			assert(body.GetShapeCount() == 1);
//			assert(body.OwnsShape(shape.GetHandle()));
//		}
//	}
//
//	if (body.IsValid())
//	{
//		body.Destroy();
//	}
//}
//
//void CleanupB2ComponentsSharedOwnership(impl::ComponentManager& componentManager, 
//										const EntitiesToDestroy& entitiesToDestroy)
//{
//	const auto& [entityIds, firstEntityIdentity] = entitiesToDestroy;
//
//	assert(entityIds.size() > 1);
//	assert(firstEntityIdentity == Identity::Parent);
//
//	std::vector<B2Body> allBodies;
//	std::unordered_set<B2Shape> allShapes;
//
//	// gather all our bodies and the shapes registered to them
//	for (int i = 0; i < entityIds.size(); i++)
//	{
//		auto entity = entityIds[i];
//
//		if (!componentManager.HasComponent<RigidBody>(entity))
//		{
//			continue;
//		}
//
//		auto& rigidBody = componentManager.GetComponent<RigidBody>(entity);
//		auto& body = bodyAccessor(rigidBody.body);
//
//		if (body.IsValid())
//		{
//			auto shapes = body.GetShapes();
//			if (!shapes.empty())
//			{
//				if (i > 0) // is child
//				{ 
//					// this failing would mean the child also has collider children somehow
//					assert(shapes.size() <= 1);
//				}
//
//				allShapes.insert(std::make_move_iterator(shapes.begin()),
//								 std::make_move_iterator(shapes.end()));
//				allBodies.push_back(std::move(body));
//			}
//		}
//	}
//
//	// look through our colliders, make sure all shapes gathered from bodies are accounted for
//	for (const auto& entity : entityIds)
//	{
//		if (!componentManager.HasComponent<Collider>(entity))
//		{
//			continue;
//		}
//
//		auto& collider = componentManager.GetComponent<Collider>(entity);
//		const auto& shape = collider.shape.GetData();
//
//		if (shape.IsValid())
//		{
//			allShapes.erase(shape);
//		}
//	}
//
//	// if not empty, there are colliders owned by entities that are unnaccounted for
//	assert(allShapes.empty());
//
//	for (auto& body : allBodies)
//	{
//		body.Destroy();
//	}
//}
//
//void UnlinkChildFromParent(impl::ComponentManager& componentManager, Entity_t child)
//{
//	Entity_t parent = componentManager.GetComponent<Parent>(child).entityId;
//	assert(componentManager.HasComponent<Children>(parent));
//
//	auto& childrenOfParent = componentManager.GetComponent<Children>(parent).childEntityIds;
//	assert(childrenOfParent.contains(parent));
//
//	childrenOfParent.erase(parent);
//
//	if (childrenOfParent.empty())
//	{
//		componentManager.RemoveComponent<Children>(parent);
//	}
//}
//
//EntitiesToDestroy GetAllEntitiesToDestroy(impl::ComponentManager& componentManager, Entity_t entityId)
//{
//	auto identity = GetEntityIdentity(componentManager, entityId);
//
//	EntitiesToDestroy entitiesToDestroy{ 
//		.entityIds = { entityId }, 
//		.firstEntityIdentity = identity 
//	};
//	
//	if (identity == Identity::Parent)
//	{
//		auto& children = componentManager.GetComponent<Children>(entityId).childEntityIds;
//
//		entitiesToDestroy.entityIds.reserve(children.size() + 1);
//
//		for (auto& child : children)
//		{
//			assert(componentManager.HasComponent<Parent>(child));
//			assert(componentManager.GetComponent<Parent>(child).entityId == entityId);
//
//			entitiesToDestroy.entityIds.push_back(child);
//		}
//	}
//	
//	return entitiesToDestroy;
//}

void CleanupB2Components(impl::ComponentManager& componentManager, Entity_t entity)
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

std::vector<Entity_t> GetAllEntitiesToDestroy(EntityManager& entityManager, impl::ComponentManager& componentManager, 
											  Entity_t entityId)
{
	std::vector<Entity_t> entitiesToDestroy{ entityId };

	// don't need to manually unlink these children since parent is getting merked
	if (EntityRelationsHelper::IsParent(entityManager, componentManager, entityId))
	{
		auto& children = EntityRelationsHelper::GetChildren(entityManager, componentManager, entityId);

		entitiesToDestroy.reserve(children.size() + 1);
		entitiesToDestroy.insert(entitiesToDestroy.end(), std::make_move_iterator(children.begin()), 
														  std::make_move_iterator(children.end()));
	}
	else if (EntityRelationsHelper::IsChild(entityManager, componentManager, entityId))
	{
		EntityRelationsHelper::UnlinkChildFromParent(entityManager, componentManager, entityId);
	}

	return entitiesToDestroy;
}

} // unnamed


void EntityDestructor::EntityDestroyed(EntityManager& entityManager, impl::ComponentManager& componentManager,
									   Entity_t entityId)
{
	auto entitiesToDestroy = GetAllEntitiesToDestroy(entityManager, componentManager, entityId);

	for (auto entityToDestroy : entitiesToDestroy)
	{
		CleanupB2Components(componentManager, entityToDestroy);

		entityManager.DestroyEntity(entityToDestroy);

		componentManager.EntityDestroyed(entityToDestroy);
	}
}



//auto entitiesToDestroy = GetAllEntitiesToDestroy(componentManager, entityId);
	//
	//const auto& [entityIds, firstEntityIdentity] = entitiesToDestroy;

	//assert(!entityIds.empty());

	//auto firstEntity = entityIds[0];

	//if (entityIds.size() == 1)
	//{
	//	CleanupB2ComponentsSingleOwnership(componentManager, firstEntity);

	//	if (firstEntityIdentity == Identity::Child)
	//	{
	//		UnlinkChildFromParent(componentManager, firstEntity);
	//	}
	//}
	//else
	//{
	//	CleanupB2ComponentsSharedOwnership(componentManager, entitiesToDestroy);
	//}