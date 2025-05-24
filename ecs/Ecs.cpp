#include "Ecs.h"
#include "EntityRelationsHelper.h"

// ENTITY DEFS //
void Entity::Destroy()
{
    assert(IsValid());

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

	return Entity{ newChild, *ecs_ };
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
	if (!IsParent())
	{
		return {};
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