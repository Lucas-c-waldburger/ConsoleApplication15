#pragma once
#include "../LuaUserType.h"
#include "../../components/NameComponent.h"

DEF_LUA_USERTYPE(Name) {
	lua.def_type("value", &Name::value);
}