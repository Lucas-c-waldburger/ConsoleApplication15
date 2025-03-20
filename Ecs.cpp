#include "Ecs.h"

void Entity::SetParent(const Entity& requestedParent)
{
    assert(IsValid());
    assert(requestedParent.IsValid());

    return ecs_->SetParent(id_, requestedParent.id_);
}

void Entity::RemoveParent()
{
    assert(IsValid());

    return ecs_->RemoveParent(id_);
}

bool Entity::AddChild(const Entity& requestedChild)
{
    assert(IsValid());
    assert(requestedChild.IsValid());

    return ecs_->AddChild(id_, requestedChild.id_);
}

bool Entity::RemoveChild(const Entity& requestedChild)
{
    assert(IsValid());
    assert(requestedChild.IsValid());

    return ecs_->RemoveChild(id_, requestedChild.id_);
}

void Entity::Destroy()
{
    assert(IsValid());

    ecs_->DestroyEntity(id_);

    id_ = kInvalidEntity;
}

bool Entity::IsValid() const
{
    return ecs_ && id_ != kInvalidEntity && ecs_->IsEntityActive(id_);
}

