#pragma once
#include "../LuaUserType.h"
#include "../../components/TransformComponent.h"

DEF_LUA_USERTYPE(Transform, Dependencies<SDL_FPoint>) {
	lua.def_type("position", &Transform::position,
				 "rotation", &Transform::rotation,
				 "scale", &Transform::scale);
}