#pragma once
#include "../LuaUserType.h"
#include "../../core/commonObjects.h"

using IDimensions = Dimensions<int>;
DEF_LUA_USERTYPE(IDimensions) {
	lua.def_type("w", &IDimensions::w,
				 "h", &IDimensions::h);
}

using FDimensions = Dimensions<float>;
DEF_LUA_USERTYPE(FDimensions) {
	lua.def_type("w", &FDimensions::w,
				 "h", &FDimensions::h);
}