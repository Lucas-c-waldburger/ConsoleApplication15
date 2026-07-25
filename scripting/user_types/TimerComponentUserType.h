#pragma once
#include "../LuaUserType.h"
#include "../../components/TimerComponent.h"

using TimerFlag = Timer::Flag;
DEF_LUA_USERTYPE(TimerFlag) {
	lua.def_enum("Active", TimerFlag::Active,
				 "RemoveOnExpiry", TimerFlag::RemoveOnExpiry);
}

DEF_LUA_USERTYPE(Timer) {
	lua.def_type("elapsed", &Timer::elapsed,
				 "duration", &Timer::duration,
				 "numRepeats", &Timer::numRepeats,
				 "flags", &Timer::flags);
}