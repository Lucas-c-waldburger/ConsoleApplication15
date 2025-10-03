#pragma once
#include <optional>
#include "BaseComponent.h"
#include "../audio/AudioHandle.h"
#include "../audio/AudioManager.h"

struct NewAudioRequest : BaseComponent<NewAudioRequest>
{
	Handle<Audio> audioHandle;
	AudioSettings settings;
	uint8_t force = 0;
};

struct AudioUpdateRequest : BaseComponent<AudioUpdateRequest>
{
	AudioInstanceID instanceId;
	AudioPlayCommand command = AudioPlayCommand::None;
	AudioSettings settings;
};

struct ActiveAudio : BaseComponent<ActiveAudio>
{
	Handle<Audio> audioHandle;
	AudioInstanceID instanceId;
	AudioStatus status = AudioStatus::Stopped;
	size_t onChannel = AudioManager::kInvalidChannelIndex;
	AudioSettings settings;
};