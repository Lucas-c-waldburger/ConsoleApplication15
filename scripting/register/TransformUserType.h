#pragma once
#include "UserTypesCommon.h"
#include "../../components/TransformComponent.h"

DEF_REGISTER_LUA_USERTYPE(Transform, "Transform",
	"position", &Transform::position,
	"rotation", &Transform::rotation,
	"scale", &Transform::scale,
	sol::meta_function::equal_to, &Transform::operator==);