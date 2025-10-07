#pragma once
#include <optional>
#include "BaseComponent.h"
#include "../audio/AudioHandle.h"
#include "../audio/AudioSettings.h"
#include "../audio/AudioInstance.h"

struct NewAudioRequest : BaseComponent<NewAudioRequest>
{
	Handle<Audio> audioHandle;
	AudioChannelSettings settings = AudioChannelSettings::Default();
	uint8_t force = 0;
};

struct AudioUpdateRequest : BaseComponent<AudioUpdateRequest>
{
	AudioInstanceID instanceId;
	AudioPlayCommand command = AudioPlayCommand::None;
	AudioUpdateSettings settings = AudioUpdateSettings::Default();
	AudioSpatialData spatialData;
};

struct ActiveAudio : BaseComponent<ActiveAudio>
{
	Handle<Audio> audioHandle;
	AudioInstanceID instanceId;
	AudioStatus status = AudioStatus::Stopped;
	size_t onChannel = std::numeric_limits<size_t>::max();
	AudioChannelSettings settings;
};