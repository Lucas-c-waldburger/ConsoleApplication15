#pragma once
#include "BaseComponent.h"
#include <sol/sol.hpp>
#include "../core/Handle.h"

struct LuaState;

struct Script : BaseComponent<Script>
{
	Handle<LuaState> stateHandle;
	sol::table table;

	bool operator==(const Script&) const = default;
};