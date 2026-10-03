#pragma once
#include "UserTypesCommon.h"
#include "../../components/ComponentConcepts.h"
#include "../../core/commonObjects.h"

DEF_REGISTER_LUA_USERTYPE(ComponentId, "ComponentId");

DEF_REGISTER_LUA_USERTYPE(Dimensions<int>, "IDimensions",
	"w", &Dimensions<int>::w,
	"h", &Dimensions<int>::h);

DEF_REGISTER_LUA_USERTYPE(Dimensions<float>, "FDimensions",
	"w", &Dimensions<float>::w,
	"h", &Dimensions<float>::h);