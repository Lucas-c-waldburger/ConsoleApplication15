#pragma once
#include "../LuaUserType.h"
#include "../LuaTypesRegistry.h"
#include "../../components/AudioComponents.h"

using AudioHandle = Handle<Audio>;
DEF_LUA_USERTYPE(AudioHandle) {
	lua.def_type(sol::meta_function::equal_to, &AudioHandle::operator==);
}

DEF_LUA_USERTYPE(AudioInstanceID) {
	lua.def_type(sol::meta_function::equal_to, &AudioInstanceID::operator==);
}

DEF_LUA_USERTYPE(AudioFadeMs) {
	lua.def_type("in", &AudioFadeMs::in, "out", &AudioFadeMs::out);
}

using Panning = HandedPair<uint8_t>;
DEF_LUA_USERTYPE(Panning) {
	lua.def_type(&Panning::left, "right", &Panning::right);
}

DEF_LUA_USERTYPE(AudioSpatialData, Dependencies<Panning>) {
	lua.def_type("angle", &AudioSpatialData::angle,
				 "distance", &AudioSpatialData::distance,
				 "panning", &AudioSpatialData::panning);
}

DEF_LUA_USERTYPE(AudioChannelSettings, Dependencies<AudioFadeMs, AudioSpatialData>) {
	lua.def_type("volume", &AudioChannelSettings::volume,
			     "loopCount", &AudioChannelSettings::loopCount,
			     "fadeMs", &AudioChannelSettings::fadeMs,
			     "spatial", &AudioChannelSettings::spatial);
}

DEF_LUA_USERTYPE(AudioUpdateSettings, Dependencies<AudioFadeMs, AudioSpatialData>) {
	lua.def_type("volume", &AudioChannelSettings::volume,
				 "loopCount", &AudioChannelSettings::loopCount,
				 "fadeMs", &AudioChannelSettings::fadeMs,
				 "spatial", &AudioChannelSettings::spatial);
}

DEF_LUA_USERTYPE(AudioPlayCommand) {
	lua.def_enum("None", AudioPlayCommand::None,
				 "Pause", AudioPlayCommand::Pause,
				 "Resume", AudioPlayCommand::Resume,
				 "Restart", AudioPlayCommand::Restart,
				 "Stop", AudioPlayCommand::Stop);
}

DEF_LUA_USERTYPE(AudioStatus) {
	lua.def_enum("Playing", AudioStatus::Playing,
				 "Paused", AudioStatus::Paused,
				 "Stopping", AudioStatus::Stopping,
				 "Stopped", AudioStatus::Stopped,
				 "Staged", AudioStatus::Staged);
}

DEF_LUA_USERTYPE(NewAudioRequest, Dependencies<AudioHandle, AudioChannelSettings>) {
	lua.def_type("audioHandle", &NewAudioRequest::audioHandle,
				 "settings", &NewAudioRequest::settings,
				 "force", &NewAudioRequest::force);
}

DEF_LUA_USERTYPE(ActiveAudio, Dependencies<AudioHandle, AudioInstanceID,
										   AudioStatus, AudioChannelSettings>) {
	lua.def_type("audioHandle", &ActiveAudio::audioHandle,
				 "instanceId", &ActiveAudio::instanceId,
				 "status", &ActiveAudio::status,
				 "onChannel", &ActiveAudio::onChannel,
				 "settings", &ActiveAudio::settings);
}
