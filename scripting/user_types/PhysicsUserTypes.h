#pragma once
#include "../LuaUserType.h"
#include "../../components/RigidBodyComponent.h"
#include "../../components/ColliderComponent.h"

using B2BodyHandle = Handle<B2Body>;
DEF_LUA_USERTYPE(B2BodyHandle) {
	lua.def_type(sol::meta_function::equal_to, [](const B2BodyHandle& lhs, const B2BodyHandle& rhs) {
		return lhs == rhs;
	});
}

using B2ShapeHandle = Handle<B2Shape>;
DEF_LUA_USERTYPE(B2ShapeHandle) {
	lua.def_type(sol::meta_function::equal_to, [](const B2ShapeHandle& lhs, const B2ShapeHandle& rhs) {
		return lhs == rhs;
	});
}