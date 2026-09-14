#include "AudioQueue.h"

bool AudioQueue::SlotOccupied(size_t slotIdx) const
{
	assert(slotIdx < 3);

	return status[slotIdx] != kEmptySlotStatus;
}

void AudioQueue::ClearSlot(size_t slotIdx)
{
	assert(slotIdx < kQueueSize);

	audioHandles[slotIdx] = {};
	instanceIds[slotIdx] = {};
	settings[slotIdx] = {};
	forcing[slotIdx] = 0;
	status[slotIdx] = kEmptySlotStatus;
}

void AudioQueue::CycleSlot(size_t slotToMove, size_t slotToOvertake)
{
	assert(slotToOvertake == slotToMove - 1);

	audioHandles[slotToOvertake] = audioHandles[slotToMove];
	instanceIds[slotToOvertake] = instanceIds[slotToMove];
	settings[slotToOvertake] = std::move(settings[slotToMove]);
	status[slotToOvertake] = status[slotToMove];
	forcing[slotToOvertake] = forcing[slotToMove];

	ClearSlot(slotToMove);
}

template <SomeMixType T>
bool AudioQueue::PlayAudio(T* activeAudioPtr)
{
	const auto& newActiveSettings = settings[0];
	const auto newForcing = forcing[0];
	auto& newStatus = status[0];

	int result = AudioController::Play(activeAudioPtr, channel,
		newActiveSettings.loopCount,
		newActiveSettings.fadeMs.in);
	if (result < 0)
	{
		LOG_ERROR("Audio play failed!");

		newStatus = AudioStatus::Stopped;

		return false;
	}

	AudioController::SetVolume(activeAudioPtr, newActiveSettings.volume);

	if constexpr (std::same_as<T, Mix_Music>)
	{
		AudioController::SetTrackPositionSec(newActiveSettings.trackPosition);
	}
	else
	{
		const auto& spatial = newActiveSettings.spatial;

		if (spatial.distance.has_value())
		{
			AudioController::SetDistance(channel, *spatial.distance);
		}
		if (spatial.panning.has_value())
		{
			AudioController::SetPanning(channel, spatial.panning->left,
				spatial.panning->right);
		}
		if (spatial.angle.has_value())
		{
			AudioController::SetSpatialPosition(channel, *spatial.angle,
				spatial.distance.value_or(0));
		}
	}

	newStatus = AudioStatus::Playing;

	if (newForcing & AudioForcing::ForcePausedAtStart)
	{
		AudioController::Pause<T>(channel);

		newStatus = AudioStatus::Paused;
	}

	return true;
}

template <SomeMixType T>
AudioStatus AudioQueue::StopAudio(T* audioPtr)
{
	assert(audioPtr);

	auto& activeSettings = settings[0];

	if (AudioController::IsPaused<T>(channel))
	{
		activeSettings.fadeMs.out = 0;
	}

	AudioController::Stop<T>(channel, activeSettings.fadeMs.out);

	if (AudioController::IsStopped<T>(channel))
	{
		if (forcing[0] & AudioForcing::ForcePauseAtEnd)
		{
			AudioController::Play(audioPtr, channel, activeSettings.loopCount,
				activeSettings.fadeMs.in);
			AudioController::Pause<T>(channel);

			activeSettings.trackPosition = 0;
		}
	}

	status[0] = AudioController::GetAudioStatus<T>(channel);

	return status[0];
}

bool AudioQueue::IsQueueIndexValidForChannel(const AudioQueueIndex& queueIdx) const
{
	return queueIdx.slot < kQueueSize && queueIdx.channel == channel;
}

AudioStatus AudioQueue::EvaluateSlotStatus(size_t slotIdx, AudioBank& audioBank,
										   const AudioQueueIndexMap& queueIdxMap)
{
	if (status[slotIdx] == kEmptySlotStatus ||
		status[slotIdx] == AudioStatus::Stopped)
	{
		return status[slotIdx];
	}

	auto it = queueIdxMap.find(instanceIds[slotIdx]);
	if (it == queueIdxMap.end() || !IsQueueIndexValidForChannel(it->second))
	{
		return (status[slotIdx] = AudioStatus::Stopped);
	}

	if (!audioBank.IsAudioValid(audioHandles[slotIdx]))
	{
		return (status[slotIdx] = AudioStatus::Stopped);
	}

	if (status[slotIdx] == AudioStatus::Staged)
	{
		return status[slotIdx];
	}

	assert(IsActiveSlot(slotIdx));

	auto audioType = audioBank.GetAudioInfo<&AudioInfo::audioType>(audioHandles[slotIdx]);
	assert(audioType.has_value());

	switch (*audioType)
	{
	case AudioType::Music:
		return (status[slotIdx] = AudioController::GetAudioStatus<Mix_Music>(channel));

	case AudioType::Sound:
		return (status[slotIdx] = AudioController::GetAudioStatus<Mix_Chunk>(channel));

	default:
		return (status[slotIdx] = AudioStatus::Stopped);
	}
}

void AudioQueue::ValidateAndCleanSlots(AudioBank& audioBank, AudioQueueIndexMap& queueIdxMap)
{
	for (size_t i = 0; i < kQueueSize; ++i)
	{
		EvaluateSlotStatus(i, audioBank, queueIdxMap);

		if (status[i] == AudioStatus::Stopped)
		{
			queueIdxMap.erase(instanceIds[i]);

			ClearSlot(i);
		}
	}
}

void AudioQueue::UpdateActiveTrackPosition(AudioBank& audioBank, float dt)
{
	if (channel == kMusicChannelIndex)
	{
		if (AudioController::IsPlaying<Mix_Music>(channel))
		{
			Mix_Music* musicPtr = audioBank.GetAudioPtr<Mix_Music>(audioHandles[0]);
			assert(musicPtr);

			settings[0].trackPosition = AudioController::GetTrackPositionSec(musicPtr);
		}
	}
	else
	{
		if (AudioController::IsPlaying<Mix_Chunk>(channel))
		{
			settings[0].trackPosition += dt;
		}
	}
}

bool AudioQueue::ShouldOvertakeSlot(size_t slotToMove, size_t slotToOvertake) const
{
	assert(slotToOvertake == slotToMove - 1);

	if (!SlotOccupied(slotToMove))
	{
		return false;
	}
	if (!SlotOccupied(slotToOvertake))
	{
		return true;
	}

	const auto incomingForce = forcing[slotToMove];
	const auto existingForce = forcing[slotToOvertake];

	if (IsActiveSlot(slotToOvertake))
	{
		if (incomingForce & (AudioForcing::ForceChannelHalt |
			AudioForcing::ForceChannelGraceful))
		{
			return true;
		}
	}
	// let stage slot 2 force overwrite stage slot 1 only if 1 wasn't also forced on
	else if ((incomingForce & AudioForcing::ForceStage) &&
		((existingForce & AudioForcing::ForceStage) == 0))
	{
		return true;
	}

	return false;
}

void AudioQueue::UpdateImpl(AudioBank& audioBank, AudioQueueIndexMap& queueIdxMap)
{
	if (ShouldOvertakeSlot(2, 1))
	{
		CycleSlot(2, 1);

		auto it = queueIdxMap.find(instanceIds[1]);
		assert(it != queueIdxMap.end());

		it->second.slot = 1;
	}

	if (ShouldOvertakeSlot(1, 0))
	{
		if (SlotOccupied(0))
		{
			if (channel == kMusicChannelIndex)
			{
				const auto status = StopAudio(audioBank.GetAudioPtr<Mix_Music>(audioHandles[0]));
				assert(status == AudioStatus::Stopped);
			}
			else
			{
				const auto status = StopAudio(audioBank.GetAudioPtr<Mix_Chunk>(audioHandles[0]));
				assert(status == AudioStatus::Stopped);
			}
		}

		CycleSlot(1, 0);

		auto it = queueIdxMap.find(instanceIds[0]);
		assert(it != queueIdxMap.end());

		it->second.slot = 0;
	}

	if (status[0] == AudioStatus::Staged) // wants new play
	{
		auto callPlayAudio = [&]<typename T> {
			T* audioPtr = audioBank.GetAudioPtr<T>(audioHandles[0]);
			assert(audioPtr);

			const bool success = PlayAudio(audioPtr);
			if (!success)
			{
				queueIdxMap.erase(instanceIds[0]);

				ClearSlot(0);

				UpdateImpl(audioBank, queueIdxMap);
			}
		};

		auto audioType = audioBank.GetAudioInfo<&AudioInfo::audioType>(audioHandles[0]);
		assert(audioType.has_value());

		if (*audioType == AudioType::Music)
		{
			callPlayAudio.template operator() < Mix_Music > ();
		}
		else
		{
			assert(*audioType == AudioType::Sound);

			callPlayAudio.template operator() < Mix_Chunk > ();
		}
	}

	// if stage slot 1 blocked stage slot 2, but 1 just got moved to active, move 2 up
	if (ShouldOvertakeSlot(2, 1))
	{
		CycleSlot(2, 1);

		auto it = queueIdxMap.find(instanceIds[1]);
		assert(it != queueIdxMap.end());

		it->second.slot = 1;
	}
}

void AudioQueue::Update(AudioBank& audioBank, AudioQueueIndexMap& queueIdxMap, float dt)
{
	ValidateAndCleanSlots(audioBank, queueIdxMap);
	// status stopped -> status empty
	// all non-empty slots have valid audioHandle and instance

	UpdateImpl(audioBank, queueIdxMap);

	UpdateActiveTrackPosition(audioBank, dt);
}

bool AudioQueue::HasOpenSlot() const
{
	return !SlotOccupied(2) || !SlotOccupied(1) || !SlotOccupied(0);
}

AudioQueue::StageAudioOutcome 
AudioQueue::StageAudio(const Handle<Audio>& handle, const AudioInstanceID& instance,
					   AudioChannelSettings&& sets, uint8_t force)
{
	auto stageOnSlot = [&](size_t slotIdx) {
		audioHandles[slotIdx] = handle;
		instanceIds[slotIdx] = instance;
		settings[slotIdx] = std::move(sets);
		status[slotIdx] = AudioStatus::Staged;
		forcing[slotIdx] = force;
		};

	if (SlotOccupied(2))
	{
		if ((force & AudioForcing::ForceStage) &&
			(forcing[2] & AudioForcing::ForceStage) == 0)
		{
			const auto replacedInstance = instanceIds[2];

			stageOnSlot(2);

			return {
				.assignedSlot = 2,
				.replacedInstance = replacedInstance
			};
		}
		else // cant force it on or 2 isnt forcing
		{
			return { .assignedSlot = std::numeric_limits<size_t>::max() };
		}
	}
	else // 2 is open
	{
		if (!SlotOccupied(1)) // could move it up to 1
		{
			if (!SlotOccupied(0)) // could move it up to active
			{
				stageOnSlot(0);

				return { .assignedSlot = 0 };
			}
			else
			{
				stageOnSlot(1);

				return { .assignedSlot = 1 };
			}
		}
		else
		{
			stageOnSlot(2);

			return { .assignedSlot = 2 };
		}
	}
}

const Handle<Audio>& AudioQueue::GetAudioHandle(size_t slotIdx) const
{
	assert(slotIdx < kQueueSize);
	return audioHandles[slotIdx];
}

AudioStatus AudioQueue::GetAudioStatus(size_t slotIdx) const
{
	assert(slotIdx < kQueueSize);
	return status[slotIdx];
}

const AudioChannelSettings& AudioQueue::GetAudioChannelSettings(size_t slotIdx) const
{
	assert(slotIdx < kQueueSize);
	return settings[slotIdx];
}

void AudioQueue::MigrateUpdateSettings(AudioUpdateSettings&& updateSettings,
									   AudioChannelSettings& channelSettings)
{
	if (updateSettings == Null<AudioUpdateSettings>())
	{
		return;
	}

	if (updateSettings.loopCount.has_value())
	{
		channelSettings.loopCount = *updateSettings.loopCount;
	}
	if (updateSettings.volume.has_value())
	{
		channelSettings.volume = *updateSettings.volume;
	}
	if (updateSettings.fadeMs.has_value())
	{
		channelSettings.fadeMs = *updateSettings.fadeMs;
	}
	if (updateSettings.trackPosition.has_value())
	{
		channelSettings.trackPosition = *updateSettings.trackPosition;
	}
	if (updateSettings.spatial.angle.has_value())
	{
		channelSettings.spatial.angle = *updateSettings.spatial.angle;
	}
	if (updateSettings.spatial.distance.has_value())
	{
		channelSettings.spatial.distance = *updateSettings.spatial.distance;
	}
	if (updateSettings.spatial.panning.has_value())
	{
		channelSettings.spatial.panning = *updateSettings.spatial.panning;
	}
}

template <SomeMixType T>
void AudioQueue::ApplyUpdateSettingsToActiveAudio(T* audioPtr, AudioUpdateSettings&& updateSettings)
{
	if (updateSettings == Null<AudioUpdateSettings>())
	{
		return;
	}

	auto& channelSettings = settings[0];

	if (updateSettings.loopCount.has_value())
	{
		channelSettings.loopCount = *updateSettings.loopCount;
	}
	if (updateSettings.fadeMs.has_value())
	{
		channelSettings.fadeMs = *updateSettings.fadeMs;
	}
	if (updateSettings.volume.has_value())
	{
		channelSettings.volume = *updateSettings.volume;

		AudioController::SetVolume(audioPtr, channelSettings.volume);
	}
	if (updateSettings.trackPosition.has_value())
	{
		channelSettings.trackPosition = *updateSettings.trackPosition;

		if constexpr (std::same_as<T, Mix_Music>)
		{
			AudioController::SetTrackPositionSec(static_cast<double>(channelSettings.trackPosition));
		}
	}
	if (updateSettings.spatial.distance.has_value())
	{
		channelSettings.spatial.distance = *updateSettings.spatial.distance;

		if constexpr (std::same_as<T, Mix_Chunk>)
		{
			if (channelSettings.spatial.distance.has_value())
			{
				AudioController::SetDistance(channel, *channelSettings.spatial.distance);
			}
		}
	}
	if (updateSettings.spatial.panning.has_value())
	{
		channelSettings.spatial.panning = *updateSettings.spatial.panning;

		if constexpr (std::same_as<T, Mix_Chunk>)
		{
			if (channelSettings.spatial.panning.has_value())
			{
				AudioController::SetPanning(channel, channelSettings.spatial.panning->left,
					channelSettings.spatial.panning->right);
			}
		}
	}
	if (updateSettings.spatial.angle.has_value())
	{
		channelSettings.spatial.angle = *updateSettings.spatial.angle;

		if constexpr (std::same_as<T, Mix_Chunk>)
		{
			if (channelSettings.spatial.angle.has_value())
			{
				AudioController::SetSpatialPosition(channel, *channelSettings.spatial.angle,
					channelSettings.spatial.distance.value_or(0));
			}
		}
	}
}

bool AudioQueue::UpdateAudioSettings(size_t slotIdx, AudioUpdateSettings&& sets, AudioBank& audioBank)
{
	assert(slotIdx < kQueueSize);

	if (IsActiveSlot(slotIdx))
	{
		if (channel == kMusicChannelIndex)
		{
			auto* musicPtr = audioBank.GetAudioPtr<Mix_Music>(audioHandles[0]);
			if (!musicPtr)
			{
				return false;
			}

			ApplyUpdateSettingsToActiveAudio(musicPtr, std::move(sets));
		}
		else
		{
			auto* soundPtr = audioBank.GetAudioPtr<Mix_Chunk>(audioHandles[slotIdx]);
			if (!soundPtr)
			{
				return false;
			}

			ApplyUpdateSettingsToActiveAudio(soundPtr, std::move(sets));
		}
	}
	else
	{
		MigrateUpdateSettings(std::move(sets), settings[slotIdx]);
	}

	return true;
}

AudioStatus AudioQueue::ExecuteAudioPlayCommand(size_t slotIdx, AudioPlayCommand command,
												AudioBank& audioBank)
{
	assert(channel >= 0 && channel <= kMusicChannelIndex);

	if (channel == kMusicChannelIndex)
	{
		return ExecuteAudioPlayCommandImpl<Mix_Music>(slotIdx, command, audioBank);
	}
	else
	{
		return ExecuteAudioPlayCommandImpl<Mix_Chunk>(slotIdx, command, audioBank);
	}
}

template <SomeMixType T>
AudioStatus AudioQueue::ExecuteAudioPlayCommandImpl(size_t slotIdx, AudioPlayCommand command, 
													AudioBank& audioBank)
{
	assert(slotIdx < kQueueSize);

	switch (command)
	{
	case AudioPlayCommand::Stop:
	case AudioPlayCommand::Halt:
		if (!IsActiveSlot(slotIdx))
		{
			status[slotIdx] = AudioStatus::Stopped;
		}
		else if (SlotOccupied(0))
		{
			if (command == AudioPlayCommand::Halt)
			{
				settings[0].fadeMs.out = 0;
			}

			StopAudio(audioBank.GetAudioPtr<T>(audioHandles[0]));
		}

		break;

	case AudioPlayCommand::Pause:
		if (IsActiveSlot(slotIdx) && SlotOccupied(0))
		{
			AudioController::Pause<T>(channel);

			status[slotIdx] = AudioController::GetAudioStatus<T>(channel);
		}

		break;

	case AudioPlayCommand::Resume:
		if (IsActiveSlot(slotIdx) && SlotOccupied(0))
		{
			AudioController::Resume<T>(channel);

			status[slotIdx] = AudioController::GetAudioStatus<T>(channel);
		}

		break;

	case AudioPlayCommand::Restart:
		if (IsActiveSlot(slotIdx) && SlotOccupied(0))
		{
			settings[0].trackPosition = 0.0f;

			auto* audioPtr = audioBank.GetAudioPtr<T>(audioHandles[0]);

			AudioController::Restart<T>(audioPtr, channel, settings[0].loopCount,
				settings[0].fadeMs.in);

			status[slotIdx] = AudioController::GetAudioStatus<T>(channel);
		}

		break;

	case AudioPlayCommand::None:
	default:
		break;
	}

	return status[slotIdx];
}

template bool AudioQueue::PlayAudio(Mix_Music*);
template bool AudioQueue::PlayAudio(Mix_Chunk*);

template AudioStatus AudioQueue::StopAudio(Mix_Music*);
template AudioStatus AudioQueue::StopAudio(Mix_Chunk*);

template void AudioQueue::ApplyUpdateSettingsToActiveAudio(Mix_Music*, AudioUpdateSettings&&);
template void AudioQueue::ApplyUpdateSettingsToActiveAudio(Mix_Chunk*, AudioUpdateSettings&&);

template AudioStatus
AudioQueue::ExecuteAudioPlayCommandImpl<Mix_Music>(size_t, AudioPlayCommand, AudioBank&);
template AudioStatus
AudioQueue::ExecuteAudioPlayCommandImpl<Mix_Chunk>(size_t, AudioPlayCommand, AudioBank&);