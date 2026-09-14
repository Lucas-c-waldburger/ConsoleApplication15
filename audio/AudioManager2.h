#pragma once
#include "AudioChannel.h"
#include "AudioQueue.h"
#include "AudioBank.h"
#include "AudioManager.h"
#include "../core/ResourceHandle.h"
#include "../ecs/Ecs.h"
#include "../systems/System.h"


class AudioManager2
{
public:
	AudioManager2();

	AudioInstanceID StageAudio(const NewAudioRequest& req, AudioBank& audioBank);

	AudioStatus ExecuteAudioPlayCommand(const AudioInstanceID& instanceId,
		AudioPlayCommand command,
		AudioBank& audioBank);

	bool ApplyAudioUpdateSettings(const AudioInstanceID& instanceId, AudioUpdateSettings&& updateSettings,
		AudioBank& audioBank);

	void UpdateAudioChannels(AudioBank& audioBank, float dt);

	bool IsAudioInstanceValid(const AudioInstanceID& instanceId) const;

	bool UpdateActiveAudioComponent(ActiveAudio& aa);

	const ActiveAudio& GetActiveAudioForInstance(const AudioInstanceID& instanceId) const;

	void PauseAll();

	void ResumeAll();

	void HaltInstance(const AudioInstanceID& instanceId);

private:
	std::array<AudioChannel, AudioChannel::kMusicChannelIndex + 1> audioChannels_;
	size_t soundChannelPriorityIdx_ = 0;
	AudioInstanceToChannelMap audioInstanceToChannelMap_;
};