#pragma once
#include "UserTypesCommon.h"
#include "../../components/AudioComponents.h"
#include "../../audio/AudioBank.h"

DEF_REGISTER_LUA_USERTYPE(Handle<Audio>, "AudioHandle",
	sol::meta_function::equal_to, &Handle<Audio>::operator==);

DEF_REGISTER_LUA_USERTYPE(AudioInstanceID, "AudioInstanceID",
	sol::meta_function::equal_to, &AudioInstanceID::operator==);

DEF_REGISTER_LUA_USERTYPE(AudioFadeMs, "AudioFadeMs",
	"in", &AudioFadeMs::in, "out", &AudioFadeMs::out);

DEF_REGISTER_LUA_USERTYPE(HandedPair<uint8_t>, "Panning",
	"left", &HandedPair<uint8_t>::left, "right", &HandedPair<uint8_t>::right);

DEF_REGISTER_LUA_USERTYPE(AudioSpatialData, "AudioSpatialData",
	"angle", &AudioSpatialData::angle,
	"distance", &AudioSpatialData::distance,
	"panning", &AudioSpatialData::panning);

DEF_REGISTER_LUA_USERTYPE(AudioChannelSettings,"AudioChannelSettings",
	"volume", &AudioChannelSettings::volume,
	"loopCount", &AudioChannelSettings::loopCount,
	"fadeMs", &AudioChannelSettings::fadeMs,
	"spatial", &AudioChannelSettings::spatial,
	"trackPosition", &AudioChannelSettings::trackPosition);

DEF_REGISTER_LUA_USERTYPE(AudioUpdateSettings, "AudioUpdateSettings",
	"volume", &AudioUpdateSettings::volume,
	"loopCount", &AudioUpdateSettings::loopCount,
	"fadeMs", &AudioUpdateSettings::fadeMs,
	"spatial", &AudioUpdateSettings::spatial,
	"trackPosition", &AudioUpdateSettings::trackPosition);

DEF_REGISTER_LUA_ENUM(AudioPlayCommand, "AudioPlayCommand");
DEF_REGISTER_LUA_ENUM(AudioStatus, "AudioStatus");

DEF_REGISTER_LUA_USERTYPE(NewAudioRequest, "NewAudioRequest",
	"audioHandle", &NewAudioRequest::audioHandle,
	"settings", &NewAudioRequest::settings,
	"force", &NewAudioRequest::force);

DEF_REGISTER_LUA_USERTYPE(ActiveAudio, "ActiveAudio",
	"audioHandle", &ActiveAudio::audioHandle,
	"instanceId", &ActiveAudio::instanceId,
	"status", &ActiveAudio::status,
	"onChannel", &ActiveAudio::onChannel,
	"settings", &ActiveAudio::settings);

DEF_REGISTER_LUA_USERTYPE(AudioBank, "AudioBank",
	"getAudio", &AudioBank::GetAudio);