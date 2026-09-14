#pragma once
#include "AudioChannel.h"
#include "AudioInstance.h"
#include "AudioBank.h"
#include "AudioStage.h"

struct AudioQueueIndex
{
	int channel = -1;
	size_t slot = 0;

	constexpr bool IsValid() const noexcept
	{
		return channel >= 0 && channel <= MIX_CHANNELS && slot < 3;
	}
};

using AudioQueueIndexMap = std::unordered_map<AudioInstanceID, AudioQueueIndex>;

class AudioQueue
{
public:
	static constexpr int kMusicChannelIndex = MIX_CHANNELS;
	static constexpr size_t kQueueSize = 3;
	static constexpr AudioStatus kEmptySlotStatus = static_cast<AudioStatus>(-1);

	struct StageAudioOutcome
	{
		size_t assignedSlot = 0;
		AudioInstanceID replacedInstance;
	};

	AudioQueue() = default;
	explicit AudioQueue(int ch) : channel(ch) {}

	bool UpdateAudioSettings(size_t slotIdx, AudioUpdateSettings&& sets, AudioBank& audioBank);

	AudioStatus ExecuteAudioPlayCommand(size_t slotIdx, AudioPlayCommand command, AudioBank& audioBank);

	void Update(AudioBank& audioBank, AudioQueueIndexMap& queueIdxMap, float dt);

	bool HasOpenSlot() const;

	StageAudioOutcome StageAudio(const Handle<Audio>& handle,
								 const AudioInstanceID& instance,
								 AudioChannelSettings&& sets, uint8_t force);

	const Handle<Audio>& GetAudioHandle(size_t slotIdx) const;
	AudioStatus GetAudioStatus(size_t slotIdx) const;
	const AudioChannelSettings& GetAudioChannelSettings(size_t slotIdx) const;

private:
	bool SlotOccupied(size_t slotIdx) const;

	void ClearSlot(size_t slotIdx);

	void CycleSlot(size_t slotToMove, size_t slotToOvertake);

	template <SomeMixType T>
	bool PlayAudio(T* audioPtr);

	template <SomeMixType T>
	AudioStatus StopAudio(T* audioPtr);

	bool IsQueueIndexValidForChannel(const AudioQueueIndex& queueIdx) const;

	AudioStatus EvaluateSlotStatus(size_t slotIdx, AudioBank& audioBank,
								   const AudioQueueIndexMap& queueIdxMap);

	void ValidateAndCleanSlots(AudioBank& audioBank, AudioQueueIndexMap& queueIdxMap);

	void UpdateActiveTrackPosition(AudioBank& audioBank, float dt);

	bool ShouldOvertakeSlot(size_t slotToMove, size_t slotToOvertake) const;

	void UpdateImpl(AudioBank& audioBank, AudioQueueIndexMap& queueIdxMap);

	static void MigrateUpdateSettings(AudioUpdateSettings&& updateSettings,
									  AudioChannelSettings& channelSettings);

	template <SomeMixType T>
	void ApplyUpdateSettingsToActiveAudio(T* audioPtr, AudioUpdateSettings&& updateSettings);

	template <SomeMixType T>
	AudioStatus ExecuteAudioPlayCommandImpl(size_t slotIdx, AudioPlayCommand command, AudioBank& audioBank);

	static constexpr bool IsActiveSlot(size_t slotIdx)
	{
		return slotIdx == 0;
	}

	std::array<Handle<Audio>, kQueueSize> audioHandles;
	std::array<AudioInstanceID, kQueueSize> instanceIds;
	std::array<AudioChannelSettings, kQueueSize> settings;
	std::array<AudioStatus, kQueueSize> status;
	std::array<uint8_t, kQueueSize> forcing;
	int channel = -1;
};

extern template bool AudioQueue::PlayAudio(Mix_Music*);
extern template bool AudioQueue::PlayAudio(Mix_Chunk*);

extern template AudioStatus AudioQueue::StopAudio(Mix_Music*);
extern template AudioStatus AudioQueue::StopAudio(Mix_Chunk*);

extern template void AudioQueue::ApplyUpdateSettingsToActiveAudio(Mix_Music*, AudioUpdateSettings&&);
extern template void AudioQueue::ApplyUpdateSettingsToActiveAudio(Mix_Chunk*, AudioUpdateSettings&&);

extern template AudioStatus
AudioQueue::ExecuteAudioPlayCommandImpl<Mix_Music>(size_t, AudioPlayCommand, AudioBank&);
extern template AudioStatus
AudioQueue::ExecuteAudioPlayCommandImpl<Mix_Chunk>(size_t, AudioPlayCommand, AudioBank&);