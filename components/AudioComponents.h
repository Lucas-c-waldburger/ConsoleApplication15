#pragma once
#include <optional>
#include "ComponentConcepts.h"
#include "../core/ResourceHandle.h"
#include "../audio/AudioSettings.h"
#include "../audio/AudioInstance.h"

struct NewAudioRequest
{
	Handle<Audio> audioHandle;
	AudioChannelSettings settings;
	uint8_t force = 0;
	float timeInQueue = 0.0f;

	bool operator==(const NewAudioRequest&) const = default;
};

struct AudioUpdateRequest
{
	AudioInstanceID instanceId;
	AudioPlayCommand command = AudioPlayCommand::None;
	AudioUpdateSettings settings;
	AudioSpatialData spatialData;

	bool operator==(const AudioUpdateRequest&) const = default;
};

struct ActiveAudio
{
	Handle<Audio> audioHandle;
	AudioInstanceID instanceId;
	AudioStatus status = AudioStatus::Stopped;
	size_t onChannel = std::numeric_limits<size_t>::max();
	AudioChannelSettings settings;

	bool operator==(const ActiveAudio&) const = default;
};