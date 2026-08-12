#pragma once
#include "AudioUserType.h"
#include "../../audio/AudioBank.h"

DEF_LUA_USERTYPE(AudioBank, Dependencies<AudioHandle>) {
	lua.def_type("getAudio", &AudioBank::GetAudio);
}