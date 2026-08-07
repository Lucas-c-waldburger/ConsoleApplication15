#pragma once
#include "../ScriptComponent.h"
#include "../RigidBodyComponent.h"
#include "../ColliderComponent.h"

inline bool ScriptValid(const Script& script)
{
	return script.table.IsValid();
}

inline bool RigidBodyValid(const RigidBody& rb)
{
	return rb.body.GetData().IsValid();
}

inline bool ColliderValid(const Collider& col)
{
	return col.shape.GetData().IsValid();
}