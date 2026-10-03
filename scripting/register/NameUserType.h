#pragma once
#include "UserTypesCommon.h"
#include "../../components/NameComponent.h"

DEF_REGISTER_LUA_USERTYPE(Name, "Name", "value", &Name::value);


