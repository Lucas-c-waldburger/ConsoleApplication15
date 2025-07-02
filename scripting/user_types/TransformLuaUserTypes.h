#pragma once
#include "../LuaTypesRegistry.h"
#include "../../components/TransformComponent.h"


// TRANSFORM
template <> inline void RegisterLuaUserType<Transform>(sol::state& lua)
{
	if (!lua["Transform"].valid())
	{
		lua.new_usertype<Transform>("Transform",
			"position", &Transform::position,
			"rotation", &Transform::rotation,
			"scale", &Transform::scale);
	}
}