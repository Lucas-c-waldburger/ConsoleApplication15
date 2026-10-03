#pragma once
#include "UserTypesCommon.h"
#include "../../components/TimerComponent.h"

DEF_REGISTER_LUA_ENUM(Timer::Flag, "TimerFlag");

DEF_REGISTER_LUA_USERTYPE(Timer, "Timer",
	"elapsed", &Timer::elapsed,
	"duration", &Timer::duration,
	"numRepeats", &Timer::numRepeats,
	"flags", &Timer::flags);