#include "AudioChannel.h"

AudioStatus AudioChannel::Update(const AudioInstanceToChannelMap& instanceMap, 
								 AudioBank& audioBank, float dt)
{
	if (IsOccupied() && !IsActiveAudioValid(instanceMap, audioBank))
	{
		ClearActiveAudio();
	}

	if (!IsOccupied())
	{
		return activeAudio_.status;
	}

	if (GetChannelIndex() == kMusicChannelIndex)
	{
		activeAudio_.status = AudioController::GetAudioStatus<Mix_Music>(GetChannelIndex());
	}
	else
	{
		activeAudio_.status = AudioController::GetAudioStatus<Mix_Chunk>(GetChannelIndex());
	}

	UpdateActiveTrackPosition(audioBank, dt);

	return activeAudio_.status;
}

const ActiveAudio& AudioChannel::GetActiveAudio() const noexcept
{
	return activeAudio_;
}

bool AudioChannel::IsActiveAudioValid(const AudioInstanceToChannelMap& instanceMap, 
									  const AudioBank& audioBank) const
{
	auto it = instanceMap.find(activeAudio_.instanceId);

	return it != instanceMap.end() && it->second == activeAudio_.onChannel &&
		audioBank.IsAudioValid(activeAudio_.audioHandle);
}

bool AudioChannel::IsOccupied() const noexcept
{
	return activeAudio_.status != kChannelEmptyStatus &&
		   activeAudio_.status != AudioStatus::Stopped;
}

template <SomeMixType T>
bool AudioChannel::PlayAudio(T* audioPtr, const AudioChannelSettings& settings, uint8_t force)
{
	assert(audioPtr);

	int result = AudioController::Play(audioPtr, GetChannelIndex(),
		settings.loopCount,
		settings.fadeMs.in);
	if (result < 0)
	{
		LOG_ERROR("Audio play failed!");

		activeAudio_.status = kChannelEmptyStatus;

		return false;
	}

	AudioController::SetVolume(audioPtr, settings.volume);

	if constexpr (std::same_as<T, Mix_Music>)
	{
		AudioController::SetTrackPositionSec(settings.trackPosition);
	}
	else
	{
		const auto& spatial = settings.spatial;

		if (spatial.distance.has_value())
		{
			AudioController::SetDistance(GetChannelIndex(), *spatial.distance);
		}
		if (spatial.panning.has_value())
		{
			AudioController::SetPanning(GetChannelIndex(), spatial.panning->left,
				spatial.panning->right);
		}
		if (spatial.angle.has_value())
		{
			AudioController::SetSpatialPosition(GetChannelIndex(), *spatial.angle,
				spatial.distance.value_or(0));
		}
	}

	if (force & AudioForcing::ForcePausedAtStart)
	{
		AudioController::Pause<T>(GetChannelIndex());
	}

	activeAudio_.status = AudioController::GetAudioStatus<T>(GetChannelIndex());

	return true;
}

void AudioChannel::ClearActiveAudio()
{
	activeAudio_.audioHandle = {};
	activeAudio_.instanceId = {};
	activeAudio_.status = kChannelEmptyStatus;
}

template <SomeMixType T>
bool AudioChannel::StopAudio()
{
	AudioController::Stop<T>(GetChannelIndex(), activeAudio_.settings.fadeMs.out);

	activeAudio_.status = AudioController::GetAudioStatus<T>(GetChannelIndex());

	return activeAudio_.status == AudioStatus::Stopped;
}

void AudioChannel::SetNewActiveAudio(const NewAudioRequest& req)
{
	activeAudio_.onChannel = activeAudio_.onChannel;
	activeAudio_.audioHandle = req.audioHandle;
	activeAudio_.instanceId = AudioInstanceID::Create();
	activeAudio_.settings = req.settings;
}

AudioChannel::StageAudioOutcome AudioChannel::StageAudio(const NewAudioRequest& req,
														 const AudioInstanceToChannelMap& instanceMap,
														 const AudioBank& audioBank)
{
	if (IsOccupied() && !IsActiveAudioValid(instanceMap, audioBank))
	{
		ClearActiveAudio();
	}

	if (GetChannelIndex() == static_cast<int>(kMusicChannelIndex))
	{
		Mix_Music* musicPtr = audioBank.GetAudioPtr<Mix_Music>(req.audioHandle);
		assert(musicPtr);

		return StageAudioImpl(musicPtr, req, instanceMap);
	}
	else
	{
		Mix_Chunk* soundPtr = audioBank.GetAudioPtr<Mix_Chunk>(req.audioHandle);
		assert(soundPtr);

		return StageAudioImpl(soundPtr, req, instanceMap);
	}
}

template <SomeMixType T>
AudioChannel::StageAudioOutcome AudioChannel::StageAudioImpl(T* audioPtr, const NewAudioRequest& req,
															 const AudioInstanceToChannelMap& instanceMap)
{
	assert(audioPtr);
	if constexpr (std::same_as<T, Mix_Music>)
	{
		assert(activeAudio_.onChannel == kMusicChannelIndex);
	}
	else
	{
		assert((activeAudio_.onChannel >= 0 &&
			activeAudio_.onChannel < kMusicChannelIndex));
	}

	StageAudioOutcome outcome{};

	if (IsOccupied())
	{
		if (req.force & (AudioForcing::ForceChannelHalt | AudioForcing::ForceChannelGraceful))
		{
			if ((req.force & AudioForcing::ForceChannelHalt) ||
				activeAudio_.status == AudioStatus::Paused)
			{
				activeAudio_.settings.fadeMs.out = 0;
			}

			if (StopAudio<T>())
			{
				if (PlayAudio(audioPtr, req.settings, req.force))
				{
					outcome.replacedInstance = activeAudio_.instanceId;

					SetNewActiveAudio(req);

					outcome.assignedInstance = activeAudio_.instanceId;
				}
			}
		}
	}
	else if (PlayAudio(audioPtr, req.settings, req.force))
	{
		SetNewActiveAudio(req);

		outcome.assignedInstance = activeAudio_.instanceId;
	}

	return outcome;
}

void AudioChannel::UpdateActiveTrackPosition(AudioBank& audioBank, float dt)
{
	if (activeAudio_.status == AudioStatus::Playing)
	{
		if (activeAudio_.onChannel == kMusicChannelIndex)
		{
			Mix_Music* musicPtr = audioBank.GetAudioPtr<Mix_Music>(activeAudio_.audioHandle);
			assert(musicPtr);

			activeAudio_.settings.trackPosition =
				static_cast<float>(AudioController::GetTrackPositionSec(musicPtr));
		}
		else
		{
			activeAudio_.settings.trackPosition += dt;
		}
	}
}

bool AudioChannel::ApplyUpdateSettingsToActiveAudio(AudioUpdateSettings&& updateSettings,
													const AudioInstanceToChannelMap& instanceMap,
													AudioBank& audioBank)
{
	if (IsOccupied() && !IsActiveAudioValid(instanceMap, audioBank))
	{
		ClearActiveAudio();
	}

	if (!IsOccupied())
	{
		return false;
	}

	if (static_cast<size_t>(GetChannelIndex()) == kMusicChannelIndex)
	{
		Mix_Music* musicPtr = audioBank.GetAudioPtr<Mix_Music>(activeAudio_.audioHandle);
		assert(musicPtr);

		ApplyUpdateSettingsToActiveAudioImpl(musicPtr, std::move(updateSettings));
	}
	else
	{
		Mix_Chunk* soundPtr = audioBank.GetAudioPtr<Mix_Chunk>(activeAudio_.audioHandle);
		assert(soundPtr);

		ApplyUpdateSettingsToActiveAudioImpl(soundPtr, std::move(updateSettings));
	}

	return true;
}


template <SomeMixType T>
void AudioChannel::ApplyUpdateSettingsToActiveAudioImpl(T* audioPtr, AudioUpdateSettings&& updateSettings)
{
	if (updateSettings == Null<AudioUpdateSettings>())
	{
		return;
	}

	if (updateSettings.loopCount.has_value())
	{
		activeAudio_.settings.loopCount = *updateSettings.loopCount;
	}
	if (updateSettings.fadeMs.has_value())
	{
		activeAudio_.settings.fadeMs = *updateSettings.fadeMs;
	}
	if (updateSettings.volume.has_value())
	{
		activeAudio_.settings.volume = *updateSettings.volume;

		AudioController::SetVolume(audioPtr, activeAudio_.settings.volume);
	}
	if (updateSettings.trackPosition.has_value())
	{
		activeAudio_.settings.trackPosition = *updateSettings.trackPosition;

		if constexpr (std::same_as<T, Mix_Music>)
		{
			AudioController::SetTrackPositionSec(
				static_cast<double>(activeAudio_.settings.trackPosition));
		}
	}
	if (updateSettings.spatial.distance.has_value())
	{
		activeAudio_.settings.spatial.distance = *updateSettings.spatial.distance;

		if constexpr (std::same_as<T, Mix_Chunk>)
		{
			if (activeAudio_.settings.spatial.distance.has_value())
			{
				AudioController::SetDistance(GetChannelIndex(),
					*activeAudio_.settings.spatial.distance);
			}
		}
	}
	if (updateSettings.spatial.panning.has_value())
	{
		activeAudio_.settings.spatial.panning = *updateSettings.spatial.panning;

		if constexpr (std::same_as<T, Mix_Chunk>)
		{
			if (activeAudio_.settings.spatial.panning.has_value())
			{
				AudioController::SetPanning(GetChannelIndex(),
					activeAudio_.settings.spatial.panning->left,
					activeAudio_.settings.spatial.panning->right);
			}
		}
	}
	if (updateSettings.spatial.angle.has_value())
	{
		activeAudio_.settings.spatial.angle = *updateSettings.spatial.angle;

		if constexpr (std::same_as<T, Mix_Chunk>)
		{
			if (activeAudio_.settings.spatial.angle.has_value())
			{
				AudioController::SetSpatialPosition(GetChannelIndex(),
					*activeAudio_.settings.spatial.angle,
					activeAudio_.settings.spatial.distance.value_or(0));
			}
		}
	}
}

AudioStatus AudioChannel::ExecuteAudioPlayCommand(AudioPlayCommand command, AudioBank& audioBank)
{
	if (GetChannelIndex() == static_cast<int>(kMusicChannelIndex))
	{
		return ExecuteAudioPlayCommandImpl<Mix_Music>(command, audioBank);
	}
	else
	{
		return ExecuteAudioPlayCommandImpl<Mix_Chunk>(command, audioBank);
	}
}

void AudioChannel::PauseDirect()
{
	if (!IsOccupied())
	{
		return;
	}

	if (GetChannelIndex() == static_cast<int>(kMusicChannelIndex))
	{
		AudioController::Pause<Mix_Music>(GetChannelIndex());

		activeAudio_.status = AudioController::GetAudioStatus<Mix_Music>(GetChannelIndex());
	}
	else
	{
		AudioController::Pause<Mix_Chunk>(GetChannelIndex());

		activeAudio_.status = AudioController::GetAudioStatus<Mix_Chunk>(GetChannelIndex());
	}
}

void AudioChannel::ResumeDirect()
{
	if (!IsOccupied())
	{
		return;
	}

	if (GetChannelIndex() == static_cast<int>(kMusicChannelIndex))
	{
		AudioController::Resume<Mix_Music>(GetChannelIndex());

		activeAudio_.status = AudioController::GetAudioStatus<Mix_Music>(GetChannelIndex());
	}
	else
	{
		AudioController::Resume<Mix_Chunk>(GetChannelIndex());

		activeAudio_.status = AudioController::GetAudioStatus<Mix_Chunk>(GetChannelIndex());
	}
}

void AudioChannel::HaltDirect()
{
	if (!IsOccupied())
	{
		return;
	}

	if (GetChannelIndex() == static_cast<int>(kMusicChannelIndex))
	{
		AudioController::Stop<Mix_Music>(GetChannelIndex(), 0);

		activeAudio_.status = AudioController::GetAudioStatus<Mix_Music>(GetChannelIndex());
	}
	else
	{
		AudioController::Stop<Mix_Chunk>(GetChannelIndex(), 0);

		activeAudio_.status = AudioController::GetAudioStatus<Mix_Chunk>(GetChannelIndex());
	}
}

template <SomeMixType T>
AudioStatus AudioChannel::ExecuteAudioPlayCommandImpl(AudioPlayCommand command, AudioBank& audioBank)
{
	if (!IsOccupied())
	{
		return activeAudio_.status;
	}

	switch (command)
	{
	case AudioPlayCommand::Stop:
	case AudioPlayCommand::Halt:

		if (command == AudioPlayCommand::Halt)
		{
			activeAudio_.settings.fadeMs.out = 0;
		}

		StopAudio<T>();

		break;

	case AudioPlayCommand::Pause:

		AudioController::Pause<T>(GetChannelIndex());

		break;

	case AudioPlayCommand::Resume:

		AudioController::Resume<T>(GetChannelIndex());

		break;

	case AudioPlayCommand::Restart:

		activeAudio_.settings.trackPosition = 0.0f;

		AudioController::Restart<T>(audioBank.GetAudioPtr<T>(activeAudio_.audioHandle),
			GetChannelIndex(),
			activeAudio_.settings.loopCount,
			activeAudio_.settings.fadeMs.in);

		break;

	case AudioPlayCommand::None:
	default:
		break;
	}

	activeAudio_.status = AudioController::GetAudioStatus<T>(GetChannelIndex());

	return activeAudio_.status;
}

template bool AudioChannel::PlayAudio(Mix_Music*, const AudioChannelSettings&, uint8_t);
template bool AudioChannel::PlayAudio(Mix_Chunk*, const AudioChannelSettings&, uint8_t);

template bool AudioChannel::StopAudio<Mix_Music>();
template bool AudioChannel::StopAudio<Mix_Chunk>();

template AudioChannel::StageAudioOutcome
AudioChannel::StageAudioImpl(Mix_Music*, const NewAudioRequest&, const AudioInstanceToChannelMap&);
template AudioChannel::StageAudioOutcome
AudioChannel::StageAudioImpl(Mix_Chunk*, const NewAudioRequest&, const AudioInstanceToChannelMap&);

template void AudioChannel::ApplyUpdateSettingsToActiveAudioImpl(Mix_Music*, AudioUpdateSettings&&);
template void AudioChannel::ApplyUpdateSettingsToActiveAudioImpl(Mix_Chunk*, AudioUpdateSettings&&);

template AudioStatus
AudioChannel::ExecuteAudioPlayCommandImpl<Mix_Music>(AudioPlayCommand, AudioBank&);
template AudioStatus
AudioChannel::ExecuteAudioPlayCommandImpl<Mix_Chunk>(AudioPlayCommand, AudioBank&);