#pragma once
#include "UserTypesCommon.h"
#include "../../components/RigidBodyComponent.h"
#include "../../components/ColliderComponent.h"

DEF_REGISTER_LUA_USERTYPE(Handle<B2Body>, "BodyHandle",
	sol::meta_function::equal_to, [](const Handle<B2Body>& lhs, const Handle<B2Body>& rhs) {
		return lhs == rhs;
	});

DEF_REGISTER_LUA_USERTYPE(Handle<B2Shape>, "ShapeHandle",
	sol::meta_function::equal_to, [](const Handle<B2Shape>& lhs, const Handle<B2Shape>& rhs) {
		return lhs == rhs;
	});