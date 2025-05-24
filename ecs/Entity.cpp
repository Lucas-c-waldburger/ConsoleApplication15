//#include "Entity.h"
//#include "EntityRelations.h"
//
//EntityRelations Entity::GetRelations()
//{
//	assert(IsValid());
//
//	return EntityRelations{ *this };
//}
//
//void Entity::Destroy()
//{
//    assert(IsValid());
//
//    ecs_->DestroyEntity(id_);
//
//    id_ = kInvalidEntity;
//}
//
//bool Entity::IsValid() const
//{
//    return ecs_ && id_ != kInvalidEntity && ecs_->IsEntityActive(id_);
//}
//
//// Entity Relations
//bool EntityRelations::SetParent(const Entity& requestedParent)
//{
//	assert(IsValid());
//	assert(requestedParent.IsValid());
//
//	return ecs_->SetParent(id_, requestedParent.GetID());
//}
//
//bool EntityRelations::RemoveParent()
//{
//	assert(IsValid());
//
//	return ecs_->RemoveParent(id_);
//}
//
//bool EntityRelations::AddChild(const Entity& requestedChild)
//{
//	assert(IsValid());
//	assert(requestedChild.IsValid());
//
//	return ecs_->AddChild(id_, requestedChild.GetID());
//}
//
//bool EntityRelations::RemoveChild(const Entity& requestedChild)
//{
//	assert(IsValid());
//	assert(requestedChild.IsValid());
//
//	return ecs_->RemoveChild(id_, requestedChild.GetID());
//}
//
//bool EntityRelations::IsParent() const
//{
//	return HasComponent<Children>();
//}
//
//bool EntityRelations::IsChild() const
//{
//	return HasComponent<Parent>();
//}
//
//Result<Entity> EntityRelations::GetParent()
//{
//	assert(IsValid());
//
//	if (HasComponent<Children>())
//	{
//		assert(!HasComponent<Parent>());
//		LOG_WARNING("Entity is not a child. Returning itself...");
//
//		return Entity{ *this };
//	}
//
//	if (!HasComponent<Parent>())
//	{
//		return MAKE_ERROR("Entity does not have a parent component");
//	}
//
//	const auto& parent = GetComponent<Parent>();
//
//	if (!ecs_->IsEntityActive(parent.entityId))
//	{
//		return Entity{};
//	}
//
//	return Entity{ parent.entityId, *ecs_ };
//}
//
//Result<std::vector<Entity>> EntityRelations::GetChildren()
//{
//	assert(IsValid());
//
//	if (HasComponent<Parent>())
//	{
//		assert(!HasComponent<Children>());
//
//		return MAKE_ERROR("Entity is not a parent");
//	}
//
//	const auto& children = GetComponent<Children>().childEntities;
//
//	std::vector<Entity> childEntities;
//	childEntities.reserve(children.size());
//
//	for (const auto& child : children)
//	{
//		if (!ecs_->IsEntityActive(child.entityId));
//		{
//			continue;
//		}
//
//		childEntities.push_back(Entity{ child.entityId, *ecs_ });
//	}
//
//	return childEntities;
//}
//
