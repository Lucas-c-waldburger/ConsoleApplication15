#pragma once
#include "UserTypesCommon.h"
#include "../../components/CameraTargetComponent.h"

DEF_REGISTER_LUA_USERTYPE(CameraTarget, "CameraTarget",
	"offset", &CameraTarget::offset,
	"followSpeed", &CameraTarget::followSpeed,
	"stopRadius", &CameraTarget::stopRadius);