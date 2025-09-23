#include "Ecs.h"
#include "EntityDestructor.h"
#include "EntityRelationsHelper.h"
#include "../core/Algorithms.h"

// ENTITY DEFS //
void Entity::Destroy()
{
    //assert(IsValid());
	if (!IsValid())
	{
		return;
	}

    ecs_->DestroyEntity(id_);

    id_ = kInvalidEntity;
}

bool Entity::IsValid() const
{
    return ecs_ && ecs_->IsEntityValid(id_);
}

EntityRelations Entity::GetRelations()
{
	return EntityRelations{ *this };
}

// ENTITY RELATION DEFS //
Entity EntityRelations::AddChild()
{
	assert(ecs_);

	if (IsChild())
	{
		return {};
	}

	auto newChild = EntityRelationsHelper::AddChild(ecs_->GetEntityManager(),
													ecs_->GetComponentManager(), id_);

	ecs_->AddComponent<EntityFlags>(newChild);

	return Entity{ newChild, *ecs_ };
}

Entity EntityRelations::AddChild(std::string_view childName)
{
	assert(ecs_);

	if (IsChild())
	{
		return {};
	}

	auto nameTag = Tag::Compose(TagCategory::kChildName, childName);

	if (!IsParent())
	{
		ecs_->componentManager_.AddComponent<Children>(id_);
	}

	const auto& existingChildren = EntityRelationsHelper::GetChildren(ecs_->GetEntityManager(),
																	  ecs_->GetComponentManager(), 
																	  id_);
	for (const auto& child : existingChildren)
	{
		if (ecs_->componentManager_.HasComponent<Tags>(child) &&
			ecs_->componentManager_.GetComponent<Tags>(child).tags.contains(nameTag))
		{
			LOG_ERROR_FMT("Child already exists with child name '{}'", childName);
			return {};
		}
	}

	auto newChild = EntityRelationsHelper::AddChild(ecs_->GetEntityManager(),
												    ecs_->GetComponentManager(), id_);

	ecs_->AddComponent<EntityFlags>(newChild);

	auto& tags = ecs_->AddComponent<Tags>(newChild).tags;
	tags.emplace(std::move(nameTag));

	return Entity{ newChild, *ecs_ };
}

Entity EntityRelations::AddProxyChild()
{
	auto newChildEntity = AddChild();

	newChildEntity.SetComponentVisibility(false);

	return newChildEntity;
}

Entity EntityRelations::AddProxyChild(std::string_view childName)
{
	auto newChildEntity = AddChild(childName);

	newChildEntity.SetComponentVisibility(false);

	return newChildEntity;
}

bool EntityRelations::IsParent() const
{
	assert(ecs_);

	return EntityRelationsHelper::IsParent(ecs_->GetEntityManager(), 
										   ecs_->GetComponentManager(), id_);
}

bool EntityRelations::IsChild() const
{
	assert(ecs_);

	return EntityRelationsHelper::IsChild(ecs_->GetEntityManager(),
								          ecs_->GetComponentManager(), id_);
}

bool EntityRelations::IsParentOf(Entity_t child) const
{
	assert(ecs_);

	return EntityRelationsHelper::IsParentOf(ecs_->GetEntityManager(),
											 ecs_->GetComponentManager(), id_, child);
}

bool EntityRelations::IsParentOf(const Entity& child) const
{
	return IsParentOf(child.GetID());
}

bool EntityRelations::IsChildOf(Entity_t parent) const
{
	assert(ecs_);

	return EntityRelationsHelper::IsChildOf(ecs_->GetEntityManager(),
										    ecs_->GetComponentManager(), id_, parent);
}

bool EntityRelations::IsChildOf(const Entity& parent) const
{
	return IsChildOf(parent.GetID());
}

bool EntityRelations::HasChildren() const
{
	return IsParent() && 
		   !EntityRelationsHelper::GetChildren(ecs_->GetEntityManager(), 
											   ecs_->GetComponentManager(), 
											   id_).empty();
}

Entity EntityRelations::GetParent()
{
	if (!IsChild())
	{
		return {};
	}

	auto parent = EntityRelationsHelper::GetParent(ecs_->GetEntityManager(),
												   ecs_->GetComponentManager(), id_);

	return Entity{ parent, *ecs_ };
}

std::vector<Entity> EntityRelations::GetChildren()
{
	if (IsChild())
	{
		return {};
	}

	if (!IsParent())
	{
		ecs_->AddComponent<Children>(id_);
	}

	const auto& children = EntityRelationsHelper::GetChildren(ecs_->GetEntityManager(),
															  ecs_->GetComponentManager(), id_);

	std::vector<Entity> childEntities;
	childEntities.reserve(children.size());

	for (const auto child : children)
	{
		if (!ecs_->IsEntityValid(child))
		{
			continue;
		}

		childEntities.push_back(Entity{ child, *ecs_ });
	}

	return childEntities;
}

Entity EntityRelations::FindChild(Entity_t childId)
{
	if (!IsParent())
	{
		return {};
	}
	if (!ecs_->IsEntityValid(childId))
	{
		return {};
	}

	const auto& existingChildren = EntityRelationsHelper::GetChildren(ecs_->GetEntityManager(),
																	  ecs_->GetComponentManager(),
																	  id_);

	return (existingChildren.contains(childId)) ? Entity{ childId, *ecs_ } : Entity{};
}

Entity EntityRelations::FindChild(std::string_view childName)
{
	if (!IsParent())
	{
		return {};
	}

	auto nameTag = Tag::Compose(TagCategory::kChildName, childName);

	const auto& existingChildren = EntityRelationsHelper::GetChildren(ecs_->GetEntityManager(),
																	  ecs_->GetComponentManager(),
																	  id_);
	for (const auto& child : existingChildren)
	{
		if (ecs_->componentManager_.HasComponent<Tags>(child) &&
			ecs_->componentManager_.GetComponent<Tags>(child).tags.contains(nameTag))
		{
			return { child, *ecs_ };
		}
	}

	return {};
}

ECS& ECS::Get()
{
	static std::unique_ptr<ECS> ecs;
	if (!ecs)
	{
		ecs = std::unique_ptr<ECS>(new ECS());
	}

	return *ecs;
}


Entity ECS::GetEntityByID(Entity_t id)
{
	auto& ecs = ECS::Get();

	if (!ecs.IsEntityActive(id))
	{
		return Entity{ kInvalidEntity, ecs };
	}
	return Entity{ id, ecs };
}

Entity_t ECS::CreateEntity_t()
{
	Entity_t entity = entityManager_.CreateEntity();
	componentManager_.EntityCreated(entity);

	return entity;
}

Entity ECS::CreateEntity()
{
	auto& ecs = ECS::Get();

	Entity_t newEntity = ecs.CreateEntity_t();

	ecs.componentManager_.AddComponent<EntityFlags>(newEntity);

	return Entity{ newEntity, ecs };
}

// TODO: call into systems with an "EntityDestroyed(...)" method
void ECS::DestroyEntity(Entity_t entity)
{
	assert(IsEntityValid(entity));
	EntityDestructor::EntityDestroyed(entityManager_, componentManager_, entity);
	//entityManager_.DestroyEntity(entity);
	//EntityRelationsHelper::DestroyRelationshipsWithEntity(componentManager_, entity);
	//componentManager_.EntityDestroyed(entity);
}

bool ECS::IsEntityActive(Entity_t entity) const
{
	assert(entity < kMaxEntities);

	bool activeAccordingToComponentManager =
		componentManager_.GetSignature(entity) & ActiveState::componentBit;
	bool activeAccordingToEntityManager = entityManager_.IsEntityActive(entity);

	assert(activeAccordingToComponentManager == activeAccordingToEntityManager);

	return activeAccordingToComponentManager;
}

bool ECS::IsEntityValid(Entity_t entity) const
{
	return (entity < kMaxEntities && IsEntityActive(entity));
}
