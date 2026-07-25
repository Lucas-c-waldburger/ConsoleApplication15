#pragma once
#include "../../components/CameraTargetComponent.h"
#include "SDLUsertypes.h"

DEF_LUA_USERTYPE(CameraTarget, Dependencies<SDL_FPoint>) {
	lua.def_type("offset", &CameraTarget::offset,
				 "followSpeed", &CameraTarget::followSpeed,
				 "stopRadius", &CameraTarget::stopRadius);
}