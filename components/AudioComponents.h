#pragma once
#include <optional>
#include "BaseComponent.h"
//#include "../audio/AudioHandle.h"
#include "../core/ResourceHandle.h"
#include "../audio/AudioSettings.h"
#include "../audio/AudioInstance.h"

struct NewAudioRequest : BaseComponent<NewAudioRequest>
{
	Handle<Audio> audioHandle;
	AudioChannelSettings settings;
	uint8_t force = 0;

	friend bool operator==(const NewAudioRequest& lhs, const NewAudioRequest& rhs)
	{
		return lhs.audioHandle == rhs.audioHandle && lhs.settings == rhs.settings &&
			   lhs.force == rhs.force;
	}
};

struct AudioUpdateRequest : BaseComponent<AudioUpdateRequest>
{
	AudioInstanceID instanceId;
	AudioPlayCommand command = AudioPlayCommand::None;
	AudioUpdateSettings settings;
	AudioSpatialData spatialData;

	friend bool operator==(const AudioUpdateRequest& lhs, const AudioUpdateRequest& rhs)
	{
		return lhs.instanceId == rhs.instanceId && lhs.command == rhs.command &&
			   lhs.settings == rhs.settings && lhs.spatialData == rhs.spatialData;
	}
};

struct ActiveAudio : BaseComponent<ActiveAudio>
{
	Handle<Audio> audioHandle;
	AudioInstanceID instanceId;
	AudioStatus status = AudioStatus::Stopped;
	size_t onChannel = std::numeric_limits<size_t>::max();
	AudioChannelSettings settings;

	friend bool operator==(const ActiveAudio& lhs, const ActiveAudio& rhs)
	{
		return lhs.audioHandle == rhs.audioHandle && lhs.instanceId == rhs.instanceId &&
			   lhs.status == rhs.status && lhs.onChannel == rhs.onChannel &&
			   lhs.settings == rhs.settings;
	}
};