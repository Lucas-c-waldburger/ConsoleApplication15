#pragma once
#include "UserTypesCommon.h"
#include "../../camera/Camera.h"

DEF_REGISTER_LUA_USERTYPE(Camera, "Camera", 
	"position", sol::property(&Camera::GetPosition,
							  [](Camera& cam, SDL_FPoint pos) { cam.SetPosition(pos); }),
	"zoom", sol::property(&Camera::GetZoomScale, &Camera::SetZoomScale),
	"rotation", sol::property(&Camera::GetRotation, &Camera::SetRotation));
