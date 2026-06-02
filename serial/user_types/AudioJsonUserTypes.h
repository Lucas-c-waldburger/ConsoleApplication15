#pragma once
#include "../../components/AudioComponents.h"
#include "../../audio/AudioBank.h"
#include "CoreJsonUserTypes.h"

NLOHMANN_JSON_SERIALIZE_ENUM(
	AudioType,
	{
		{ AudioType::Unknown, "Unknown" },
		{ AudioType::Sound,   "Sound" },
		{ AudioType::Music,   "Music" }
	}
)

NLOHMANN_JSON_SERIALIZE_ENUM(
	AudioPlayCommand,
	{
		{ AudioPlayCommand::None,	 "None" },
		{ AudioPlayCommand::Pause,   "Pause" },
		{ AudioPlayCommand::Resume,  "Resume" },
		{ AudioPlayCommand::Restart, "Restart" },
		{ AudioPlayCommand::Stop,    "Stop" }
	}
)

NLOHMANN_JSON_SERIALIZE_ENUM(
	AudioStatus,
	{
		{ AudioStatus::Playing,	 "Playing" },
		{ AudioStatus::Paused,   "Paused" },
		{ AudioStatus::Stopping, "Stopping" },
		{ AudioStatus::Stopped,  "Stopped" },
		{ AudioStatus::Staged,   "Staged" }
	}
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AudioSpatialData, angle, distance, panning)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AudioFadeMs, in, out)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AudioChannelSettings, volume, loopCount, fadeMs, spatial)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AudioDescriptor, audioType, name, filepath)
