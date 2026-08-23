#include "Ecs.h"
#include "EntityDestructor.h"
#include "EntityRelationsHelper.h"
#include "EntityEvents.h"
#include "EntityPhysics.h"
#include "../core/Algorithms.h"

namespace {

template <typename T>
void ClearComponentImpl(Entity& e)
{
	if constexpr (public_mutable_component_v<T>) { e.RemoveComponent<T>(); }
}

struct ClearComponentsImpl
{
	template <typename...Ts>
	static void Apply(Entity& e) { ((ClearComponentImpl<Ts>(e)), ...); }
};

} // unnamed



// ENTITY DEFS //
void Entity::Destroy()
{
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

ComponentSignature Entity::GetComponentSignature() const
{
	if (!IsValid())
	{
		return 0;
	}

	return ecs_->GetComponentManager().GetSignature(id_);
}

void Entity::ClearComponents()
{
	ComponentTypeList::template Apply<ClearComponentsImpl>(*this);
}

EntityRelations Entity::GetRelations()
{
	return EntityRelations{ *this };
}

EntityEvents Entity::GetEvents(EventBus& bus)
{
	return EntityEvents{ *this, &bus };
}

EntityPhysics Entity::GetPhysics(B2World& world)
{
	return EntityPhysics{ *this, &world };
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

Entity EntityRelations::AddChild(std::string_view childName)
{
	assert(ecs_);

	if (IsChild())
	{
		return {};
	}

	auto newChild = EntityRelationsHelper::AddChild(ecs_->GetEntityManager(),
												    ecs_->GetComponentManager(), id_);

	ecs_->AddComponent<Name>(newChild).value = childName;

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

bool EntityRelations::SetParent(Entity_t parent)
{
	assert(ecs_);

	return EntityRelationsHelper::SetParent(ecs_->GetEntityManager(),
											ecs_->GetComponentManager(), id_, parent);
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
	static ECS instance{};
	return instance;

	//static std::unique_ptr<ECS> ecs;
	//if (!ecs)
	//{
	//	ecs = std::unique_ptr<ECS>(new ECS());
	//}

	//return *ecs;
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

Entity ECS::CreateEntity(std::string_view name)
{
	auto& ecs = ECS::Get(); 

	Entity_t newEntity = ecs.CreateEntity_t();

	ecs.componentManager_.AddComponent<EntityFlags>(newEntity);
	ecs.componentManager_.AddComponent<Name>(newEntity).value = name;

	return Entity{ newEntity, ecs };
}

void ECS::DestroyEntity(Entity_t entity)
{
	assert(IsEntityValid(entity));

	EmitEntityDestroyedSignal(Entity{ entity, *this });

	entityManager_.DestroyEntity(entity);
	componentManager_.EntityDestroyed(entity);

	//EntityDestructor::EntityDestroyed(entityManager_, componentManager_, entity);

	//EntityRelationsHelper::DestroyRelationshipsWithEntity(componentManager_, entity);
}

//// TODO: Remove "ActiveState" component
bool ECS::IsEntityActive(Entity_t entity) const
{
	if (GetEntity_tIndex(entity) >= kMaxEntities)
	{
		return false;
	}

	const auto& sig = componentManager_.GetSignature(entity);

	bool activeAccordingToComponentManager = (sig & ActiveState::componentBit);
	bool activeAccordingToEntityManager = entityManager_.IsEntityActive(entity);

	//assert(activeAccordingToComponentManager == activeAccordingToEntityManager);
	//if (activeAccordingToComponentManager != activeAccordingToEntityManager)
	//{
	//	int x = 0;
	//}

	return activeAccordingToEntityManager;
}

bool ECS::IsEntityValid(Entity_t entity) const
{
	return entity != kInvalidEntity && IsEntityActive(entity);
}

bool ECS::IsEntityNameUnique(std::string_view name) const
{
	auto entities = entityManager_.GetActiveEntities();
	for (const auto e : entities)
	{
		if (componentManager_.HasComponent<Name>(e) &&
			componentManager_.GetComponent<Name>(e) == name)
		{
			return false;
		}
	}	
	return true;
}

void ECS::AddEntityName(Entity_t e, std::string_view name)
{
	auto& nameCmp = componentManager_.AddComponent<Name>(e);

	if (!IsEntityNameUnique(name))
	{
		nameCmp.value = std::format("{}_{}", name, __COUNTER__);
	}
	else
	{
		nameCmp.value = name;
	}
}

void ECS::EmitEntityDestroyedSignal(Entity&& e)
{
	entityDestroyedSignal_.Emit(e);
}

void ECS::SerializeUserComponents(nlohmann::json& j, const Entity& e)
{
	const auto& ecs = ECS::Get();

	ecs.userComponentBridge_.SerializeComponentData(j, e.GetID(), ecs.componentManager_);
}

Result<Void> ECS::DeserializeUserComponents(const nlohmann::json& j, Entity& e)
{
	auto& ecs = ECS::Get();

	return ecs.userComponentBridge_.DeserializeComponentData(j, e.GetID(), ecs.componentManager_);
}