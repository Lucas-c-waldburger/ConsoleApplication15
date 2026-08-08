#pragma once
#include "CoreLuaUserTypes.h"
#include "../../camera/Camera.h"

DEF_LUA_USERTYPE(Camera, Dependencies<SDL_FPoint>) {
	lua.def_type("position", sol::property(&Camera::GetPosition,
							 [](Camera& cam, SDL_FPoint pos) { cam.SetPosition(pos); }),
				 "zoom", sol::property(&Camera::GetZoomScale, &Camera::SetZoomScale),
				 "rotation", sol::property(&Camera::GetRotation, &Camera::SetRotation));
}