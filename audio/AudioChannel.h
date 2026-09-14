#pragma once
#include <unordered_map>
#include "AudioBank.h"
#include "AudioController.h"
#include "AudioCommon.h"
#include "AudioInstance.h"
#include "AudioQueue.h"
#include "AudioStage.h"
#include "../components/AudioComponents.h"

using AudioInstanceToChannelMap = std::unordered_map<AudioInstanceID, size_t>;

class AudioChannel
{
public:
	static constexpr size_t kMusicChannelIndex = MIX_CHANNELS;
	static constexpr AudioStatus kChannelEmptyStatus = static_cast<AudioStatus>(-1);

	struct StageAudioOutcome
	{
		AudioInstanceID assignedInstance;
		AudioInstanceID replacedInstance;
	};

	AudioChannel() = default;
	explicit AudioChannel(size_t ch) : activeAudio_{ 
		.status = kChannelEmptyStatus,
		.onChannel = ch
	} {}

	AudioStatus Update(const AudioInstanceToChannelMap& instanceMap, AudioBank& audioBank, float dt);

	const ActiveAudio& GetActiveAudio() const noexcept;
	
	bool IsOccupied() const noexcept;

	void ClearActiveAudio();

	StageAudioOutcome StageAudio(const NewAudioRequest& req,
								 const AudioInstanceToChannelMap& instanceMap, 
								 const AudioBank& audioBank);

	bool ApplyUpdateSettingsToActiveAudio(AudioUpdateSettings&& updateSettings,
										  const AudioInstanceToChannelMap& instanceMap,
										  AudioBank& audioBank);

	AudioStatus ExecuteAudioPlayCommand(AudioPlayCommand command, AudioBank& audioBank);

	void PauseDirect();
	void ResumeDirect();
	void HaltDirect();

private:
	constexpr int GetChannelIndex() const noexcept
	{
		return static_cast<int>(activeAudio_.onChannel);
	}

	bool IsActiveAudioValid(const AudioInstanceToChannelMap& instanceMap,
							const AudioBank& audioBank) const;

	void SetNewActiveAudio(const NewAudioRequest& req);

	template <SomeMixType T>
	bool PlayAudio(T* audioPtr, const AudioChannelSettings& settings, uint8_t force);

	template <SomeMixType T>
	bool StopAudio();

	template <SomeMixType T>
	StageAudioOutcome StageAudioImpl(T* audioPtr, const NewAudioRequest& req,
									 const AudioInstanceToChannelMap& instanceMap);

	void UpdateActiveTrackPosition(AudioBank& audioBank, float dt);

	template <SomeMixType T>
	void ApplyUpdateSettingsToActiveAudioImpl(T* audioPtr, AudioUpdateSettings&& updateSettings);

	template <SomeMixType T>
	AudioStatus ExecuteAudioPlayCommandImpl(AudioPlayCommand command, AudioBank& audioBank);

	ActiveAudio activeAudio_;
};

extern template bool AudioChannel::PlayAudio(Mix_Music*, const AudioChannelSettings&, uint8_t);
extern template bool AudioChannel::PlayAudio(Mix_Chunk*, const AudioChannelSettings&, uint8_t);

extern template bool AudioChannel::StopAudio<Mix_Music>();
extern template bool AudioChannel::StopAudio<Mix_Chunk>();

extern template AudioChannel::StageAudioOutcome
AudioChannel::StageAudioImpl(Mix_Music*, const NewAudioRequest&, const AudioInstanceToChannelMap&);
extern template AudioChannel::StageAudioOutcome
AudioChannel::StageAudioImpl(Mix_Chunk*, const NewAudioRequest&, const AudioInstanceToChannelMap&);

extern template void AudioChannel::ApplyUpdateSettingsToActiveAudioImpl(Mix_Music*, AudioUpdateSettings&&);
extern template void AudioChannel::ApplyUpdateSettingsToActiveAudioImpl(Mix_Chunk*, AudioUpdateSettings&&);

extern template AudioStatus 
AudioChannel::ExecuteAudioPlayCommandImpl<Mix_Music>(AudioPlayCommand, AudioBank&);
extern template AudioStatus
AudioChannel::ExecuteAudioPlayCommandImpl<Mix_Chunk>(AudioPlayCommand, AudioBank&);