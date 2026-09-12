#pragma once
#include "AudioBank.h"
#include "AudioManager.h"
#include "../core/ResourceHandle.h"
#include "../ecs/Ecs.h"
#include "../systems/System.h"


class AudioChannel
{
public:
	template <SomeMixType T>
	static int GetVolume(T* audioPtr)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			return Mix_GetMusicVolume(audioPtr);
		}
		else
		{
			return Mix_VolumeChunk(audioPtr, -1);
		}
	}

	template <SomeMixType T>
	static void SetVolume(T* audioPtr, int volume)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			Mix_VolumeMusic(std::clamp(volume, 0, MIX_MAX_VOLUME));
		}
		else
		{
			Mix_VolumeChunk(audioPtr, std::clamp(volume, 0, MIX_MAX_VOLUME));
		}
	}

	template <SomeMixType T>
	static bool IsPlaying(int channelIdx)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			return Mix_PlayingMusic() != 0 && Mix_PausedMusic() == 0;
		}
		else
		{
			return Mix_Playing(channelIdx) != 0 && Mix_Paused(channelIdx) == 0;
		}
	}

	template <SomeMixType T>
	static bool IsStopped(int channelIdx)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			return Mix_PlayingMusic() == 0 && Mix_PausedMusic() == 0;
		}
		else
		{
			return Mix_Playing(channelIdx) == 0 && Mix_Paused(channelIdx) == 0;
		}
	}

	template <SomeMixType T>
	static bool IsPaused(int channelIdx)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			return Mix_PausedMusic() != 0;
		}
		else
		{
			return Mix_Paused(channelIdx) != 0;
		}
	}

	template <SomeMixType T>
	static bool IsFadingIn(int channelIdx)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			return (Mix_FadingMusic() == MIX_FADING_IN);
		}
		else
		{
			return (Mix_FadingChannel(channelIdx) == MIX_FADING_IN);
		}
	}

	template <SomeMixType T>
	static bool IsFadingOut(int channelIdx)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			return (Mix_FadingMusic() == MIX_FADING_OUT);
		}
		else
		{
			return (Mix_FadingChannel(channelIdx) == MIX_FADING_OUT);
		}
	}

	template <SomeMixType T>
	static AudioStatus GetAudioStatus(int channelIdx)
	{
		if (IsPaused<T>(channelIdx))
		{
			return AudioStatus::Paused;
		}
		if (IsFadingOut<T>(channelIdx))
		{
			return AudioStatus::Stopping;
		}
		if (IsPlaying<T>(channelIdx))
		{
			return AudioStatus::Playing;
		}

		return AudioStatus::Stopped;
	}

	template <SomeMixType T>
	static void Pause(int channelIdx)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			Mix_PauseMusic();
		}
		else
		{
			Mix_Pause(channelIdx);
		}
	}

	template <SomeMixType T>
	static void Resume(int channelIdx)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			Mix_ResumeMusic();
		}
		else
		{
			Mix_Resume(channelIdx);
		}
	}

	template <SomeMixType T>
	static int Play(T* audioPtr, int channelIdx, int loops, int fadeInMs)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			if (fadeInMs > 0)
			{
				return Mix_FadeInMusic(audioPtr, loops, fadeInMs);
			}
			else
			{
				return Mix_PlayMusic(audioPtr, loops);
			}
		}
		else
		{
			if (fadeInMs > 0)
			{
				return Mix_FadeInChannel(channelIdx, audioPtr, loops, fadeInMs);
			}
			else
			{
				return Mix_PlayChannel(channelIdx, audioPtr, loops);
			}
		}
	}

	template <SomeMixType T>
	static int Stop(int channelIdx, int fadeOutMs)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			if (fadeOutMs > 0)
			{
				return Mix_FadeOutMusic(fadeOutMs);
			}
			else
			{
				return Mix_HaltMusic();
			}
		}
		else
		{
			if (fadeOutMs > 0)
			{
				return Mix_FadeOutChannel(channelIdx, fadeOutMs);
			}
			else
			{
				return Mix_HaltChannel(channelIdx);
			}
		}
	}

	template <SomeMixType T>
	static int FadeIn(T* audioPtr, int channelIdx, int fadeInMs, int loops)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			return Mix_FadeInMusic(audioPtr, loops, fadeInMs);
		}
		else
		{
			return Mix_FadeInChannel(channelIdx, audioPtr, loops, fadeInMs);
		}
	}

	template <SomeMixType T>
	static int FadeOut(int channelIdx, int fadeOutMs)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			return Mix_FadeOutMusic(fadeOutMs);
		}
		else
		{
			return Mix_FadeOutChannel(channelIdx, fadeOutMs);
		}
	}

	template <SomeMixType T>
	static double GetTrackPositionSec(T* audioPtr)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			return Mix_GetMusicPosition(audioPtr);
		}
		else
		{
			return -1.0;
		}
	}

	static int SetTrackPositionSec(double sec)
	{
		return Mix_SetMusicPosition(sec);
	}

	static int SetPanning(int channelIdx, uint8_t left, uint8_t right)
	{
		return Mix_SetPanning(channelIdx, left, right);
	}

	static int SetDistance(int channelIdx, uint8_t distance)
	{
		return Mix_SetDistance(channelIdx, distance);
	}

	static int SetSpatialPosition(int channelIdx, int16_t angle, uint8_t distance)
	{
		return Mix_SetPosition(channelIdx, angle, distance);
	}

	/*template <SomeMixType T>
	static void RewindPaused(int channelIdx, int loops, int fadeInMs)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			Mix_RewindMusic();
			Mix_PauseMusic();
		}
		else
		{
			Mix_HaltChannel(channelIdx);

			if (fadeInMs > 0)
			{
				Mix_FadeInChannel(channelIdx, audioPtr, loops, fadeInMs);
			}
			else
			{
				Mix_PlayChannel(channelIdx, audioPtr, loops);
			}

			Mix_Pause(channelIdx);
		}
	}*/

	template <SomeMixType T>
	static void Restart(T* audioPtr, int channelIdx, int loops, int fadeInMs)
	{
		if constexpr (std::same_as<T, Mix_Music>)
		{
			Mix_RewindMusic();
		}
		else
		{
			Mix_HaltChannel(channelIdx);

			if (fadeInMs > 0)
			{
				Mix_FadeInChannel(channelIdx, audioPtr, loops, fadeInMs);
			}
			else
			{
				Mix_PlayChannel(channelIdx, audioPtr, loops);
			}
		}
	}

private:
	int channelIndex_ = -1;
};

struct QueueIndex
{
	int channel = -1;
	size_t slot = 0;

	constexpr bool IsValid() const noexcept
	{
		return channel >= 0 && channel <= MIX_CHANNELS && slot < 3;
	}
};

using QueueIndexMap = std::unordered_map<AudioInstanceID, QueueIndex>;

struct AudioQueue
{
	static constexpr int kMusicChannelIndex = MIX_CHANNELS;
	static constexpr size_t kQueueSize = 3;

	template <size_t I> requires (I < 3)
	bool SlotOccupied() const
	{
		const bool occupied = audioHandles[I].IsValid() && instanceIds[I].IsValid() &&
							  status[I] != AudioStatus::Stopped;
		if constexpr (I > 0)
		{
			if (occupied)
			{
				// make sure staged audio is either stopped or stage - no other allowed statuses
				assert(status[I] == AudioStatus::Staged);
			}
		}

		return occupied;
	}

	bool SlotOccupied(size_t slotIdx) const
	{
		assert(slotIdx < 3);

		return audioHandles[slotIdx].IsValid() && instanceIds[slotIdx].IsValid() &&
			   status[slotIdx] != AudioStatus::Stopped;
	}

	template <SomeMixType T>
	bool SlotOccupied(size_t slotIdx, const QueueIndexMap& queueIndexMap, const AudioBank& audioBank) const
	{
		assert(slotIdx < 3);

		return audioBank.IsAudioValid(audioHandles[slotIdx]) && 
			   queueIndexMap.contains(instanceIds[slotIdx]) &&
			   !AudioChannel::IsStopped<T>(channel);
	}

	void ClearSlot(size_t slotIdx)
	{
		assert(slotIdx < kQueueSize);

		audioHandles[slotIdx] = {};
		instanceIds[slotIdx] = {};
		settings[slotIdx] = {};
		forcing[slotIdx] = 0;
		status[slotIdx] = AudioStatus::Stopped;
	}

	template <SomeMixType T>
	void UpkeepSlots(QueueIndexMap& queueIndexMap, const AudioBank& audioBank)
	{
		for (size_t slotIdx = 0; slotIdx < kQueueSize; ++slotIdx)
		{
			if (!audioBank.IsAudioValid(audioHandles[slotIdx]))
			{
				queueIndexMap.erase(instanceIds[slotIdx]);
				ClearSlot(slotIdx);

				continue;
			}
			if (!queueIndexMap.contains(instanceIds[slotIdx]))
			{
				ClearSlot(slotIdx);
			}

			if (IsActiveSlot(slotIdx))
			{ 
				if (AudioChannel::IsStopped<T>(channel))
				{
					queueIndexMap.erase(instanceIds[slotIdx]);
					ClearSlot(slotIdx);
				}
			}
			else
			{
				if (status[slotIdx] == AudioStatus::Stopped)
				{
					queueIndexMap.erase(instanceIds[slotIdx]);
					ClearSlot(slotIdx);
				}
			}
		}
	}

	template <size_t A, size_t B> requires (A < kQueueSize && B < kQueueSize)
	void CycleSlot()
	{
		audioHandles[A] = audioHandles[B];
		instanceIds[A] = instanceIds[B];
		settings[A] = std::move(settings[B]);
		status[A] = status[B];
		forcing[A] = forcing[B];

		ClearSlot(B);
	}

	template <SomeMixType T>
	bool ShouldForceChannelStop(uint8_t force) const
	{
		return (force & AudioForcing::ForceChannelHalt) ||
			   ((force & AudioForcing::ForceChannelGraceful) && 
				!AudioChannel::IsFadingOut<T>(channel));
	}

	template <SomeMixType T>
	void PlayAudio(T* activeAudioPtr)
	{
		const auto& newActiveSettings = settings[0];
		const auto newForcing = forcing[0];
		auto& newStatus = status[0];

		int result = AudioChannel::Play(activeAudioPtr, channel,
										newActiveSettings.loopCount,
										newActiveSettings.fadeMs.in);
		if (result < 0)
		{
			LOG_ERROR("Audio play failed!");

			newStatus = AudioStatus::Stopped;

			return;
		}

		AudioChannel::SetVolume(activeAudioPtr, newActiveSettings.volume);

		if constexpr (std::same_as<T, Mix_Music>)
		{
			AudioChannel::SetTrackPositionSec(newActiveSettings.trackPosition);
		}
		else
		{
			const auto& spatial = newActiveSettings.spatial;

			if (spatial.distance.has_value())
			{
				AudioChannel::SetDistance(channel, *spatial.distance);
			}
			if (spatial.panning.has_value())
			{
				AudioChannel::SetPanning(channel, spatial.panning->left,
												  spatial.panning->right);
			}
			if (spatial.angle.has_value())
			{
				AudioChannel::SetSpatialPosition(channel, *spatial.angle,
														  spatial.distance.value_or(0));
			}
		}

		newStatus = AudioStatus::Playing;

		if (newForcing & AudioForcing::ForcePausedAtStart)
		{
			AudioChannel::Pause<T>(channel);

			newStatus = AudioStatus::Paused;
		}
	}
	
	template <SomeMixType T>
	void TryMoveStagedToActiveOccupied(QueueIndexMap& queueIdxMap, AudioBank& audioBank)
	{
		const auto& stagedHandle = audioHandles[1];

		auto* stagedAudioPtr = audioBank.GetAudioPtr<T>(stagedHandle);
		assert(stagedAudioPtr);

		const bool forceStop = ShouldForceChannelStop<T>(forcing[1]);
		if (forceStop)
		{
			auto& activeSettings = settings[0];
			auto& activeForcing = forcing[0];

			if (AudioChannel::IsPaused<T>(channel))
			{
				activeSettings.fadeMs.out = 0;
			}

			activeForcing &= ~AudioForcing::ForcePausedAtStart;

			AudioChannel::Stop<T>(channel, activeSettings.fadeMs.out);
		}

		if (AudioChannel::IsStopped<T>(channel))
		{
			if (!forceStop && (forcing[0] & AudioForcing::ForcePauseAtEnd))
			{
				const auto& activeHandle = audioHandles[0];
				auto& activeSettings = settings[0];
				auto& activeStatus = status[0];

				auto* activeAudioPtr = audioBank.GetAudioPtr<T>(activeHandle);
				assert(activeAudioPtr);

				AudioChannel::Play(activeAudioPtr, channel, activeSettings.loopCount, 
															activeSettings.fadeMs.in);
				AudioChannel::Pause<T>(channel);

				activeSettings.trackPosition = 0;
				activeStatus = AudioStatus::Paused;

				return;
			}

			queueIdxMap.erase(instanceIds[0]);

			CycleSlot<0, 1>();

			auto it = queueIdxMap.find(instanceIds[0]);
			assert(it != queueIdxMap.end());

			it->second.slot = 0;

			PlayAudio(stagedAudioPtr);
		}
	}

	template <SomeMixType T>
	void TryMoveStagedToActiveUnoccupied(QueueIndexMap& queueIdxMap, AudioBank& audioBank)
	{
		const auto& stagedHandle = audioHandles[1];

		auto* stagedAudioPtr = audioBank.GetAudioPtr<T>(stagedHandle);
		assert(stagedAudioPtr);

		CycleSlot<0, 1>();

		auto it = queueIdxMap.find(instanceIds[0]);
		assert(it != queueIdxMap.end());

		it->second.slot = 0;

		PlayAudio(stagedAudioPtr);
	}

	void UpdateTrackPosition(AudioBank& audioBank, float dt)
	{
		if (channel == kMusicChannelIndex)
		{
			if (AudioChannel::IsPlaying<Mix_Music>(channel))
			{
				Mix_Music* musicPtr = audioBank.GetAudioPtr<Mix_Music>(audioHandles[0]);
				assert(musicPtr);

				settings[0].trackPosition = AudioChannel::GetTrackPositionSec(musicPtr);
			}
		}
		else
		{
			if (AudioChannel::IsPlaying<Mix_Chunk>(channel))
			{
				settings[0].trackPosition += dt;
			}
		}
	}

	void UpdateSlots(QueueIndexMap& queueIdxMap, AudioBank& audioBank, float dt)
	{
		if (channel == kMusicChannelIndex)
		{
			UpkeepSlots<Mix_Music>(queueIdxMap, audioBank);
		}
		else
		{
			UpkeepSlots<Mix_Chunk>(queueIdxMap, audioBank);
		}

		// if back stage occupied but front stage unoccupied, move it up
		if (SlotOccupied<2>() && !SlotOccupied<1>())
		{
			CycleSlot<1, 2>();

			auto it = queueIdxMap.find(instanceIds[1]);
			assert(it != queueIdxMap.end());

			it->second.slot = 1;
		}

		if (SlotOccupied<1>())
		{
			// try move front of stage to active
			auto it = queueIdxMap.find(instanceIds[1]);
			assert(it != queueIdxMap.end());
			assert(it->second.channel == channel);

			if (SlotOccupied<0>())
			{
				if (channel == kMusicChannelIndex)
				{
					TryMoveStagedToActiveOccupied<Mix_Music>(queueIdxMap, audioBank);
				}
				else
				{
					TryMoveStagedToActiveOccupied<Mix_Chunk>(queueIdxMap, audioBank);
				}
			}
			else
			{
				if (channel == kMusicChannelIndex)
				{
					TryMoveStagedToActiveUnoccupied<Mix_Music>(queueIdxMap, audioBank);
				}
				else
				{
					TryMoveStagedToActiveUnoccupied<Mix_Chunk>(queueIdxMap, audioBank);
				}
			}
		}

		// check if front stage got moved to active
		// if so, see if back stage should now be moved up to front stage
		if (SlotOccupied<2>() && !SlotOccupied<1>())
		{
			CycleSlot<1, 2>();

			auto it = queueIdxMap.find(instanceIds[1]);
			assert(it != queueIdxMap.end());

			it->second.slot = 1;
		}

		UpdateTrackPosition(audioBank, dt);
	}

	struct StageAudioOutcome
	{
		size_t assignedSlot = 0;
		AudioInstanceID replacedInstance;
	};

	template <SomeMixType T>
	Result<StageAudioOutcome> StageAudio(T* audioPtr, const Handle<Audio>& handle, 
										 const AudioInstanceID& instance,
										 AudioChannelSettings&& sets, uint8_t force)
	{
		if (!audioPtr)
		{
			return MAKE_ERROR("Audio pointer was null");
		}

		if constexpr (std::same_as<T, Mix_Music>)
		{
			if (channel != kMusicChannelIndex)
			{
				return MAKE_ERROR("Queue is not of type music");
			}
		}
		else
		{
			if (channel < 0 || channel >= kMusicChannelIndex)
			{
				return MAKE_ERROR("Queue is not of type sound");
			}
		}

		auto assignToSlot = [&](size_t slotIdx, AudioStatus stat) {
			audioHandles[slotIdx] = handle;
			instanceIds[slotIdx] = instance;
			settings[slotIdx] = std::move(sets);
			status[slotIdx] = stat;
			forcing[slotIdx] = force;
		};

		if (SlotOccupied<2>())
		{
			if (force & AudioForcing::ForceStage)
			{
				const auto replacedInstance = instanceIds[2]

				ClearSlot(2);
				
				assignToSlot(2, AudioStatus::Staged);

				return StageAudioOutcome{
					.assignedSlot = 2,
					.replacedInstance = replacedInstance
				};
			}
			else
			{
				return MAKE_ERROR("Could not stage audio - queue full");
			}
		}
		else
		{
			if (!SlotOccupied<1>())
			{
				if (!SlotOccupied<0>())
				{
					assignToSlot(0, AudioStatus::Playing);

					PlayAudio(audioPtr);

					return StageAudioOutcome{
						.assignedSlot = 0
					};
				}
				else
				{
					assignToSlot(1, AudioStatus::Staged);

					return StageAudioOutcome{
						.assignedSlot = 1
					};
				}
			}
			else
			{
				assignToSlot(2, AudioStatus::Staged);

				return StageAudioOutcome{
					.assignedSlot = 2
				};
			}
		}
	}

	static void MigrateUpdateSettings(AudioUpdateSettings&& updateSettings, 
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
	void ApplyUpdateSettingsToActiveAudio(T* audioPtr, AudioUpdateSettings&& updateSettings)
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

			AudioChannel::SetVolume(audioPtr, channelSettings.volume);
		}
		if (updateSettings.trackPosition.has_value())
		{
			channelSettings.trackPosition = *updateSettings.trackPosition;

			if constexpr (std::same_as<T, Mix_Music>)
			{
				AudioChannel::SetTrackPositionSec(static_cast<double>(channelSettings.trackPosition));
			}
		}
		if (updateSettings.spatial.distance.has_value())
		{
			channelSettings.spatial.distance = *updateSettings.spatial.distance;

			if constexpr (std::same_as<T, Mix_Chunk>)
			{
				if (channelSettings.spatial.distance.has_value())
				{
					AudioChannel::SetDistance(channel, *channelSettings.spatial.distance);
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
					AudioChannel::SetPanning(channel, channelSettings.spatial.panning->left,
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
					AudioChannel::SetSpatialPosition(channel, *channelSettings.spatial.angle,
													 channelSettings.spatial.distance.value_or(0));
				}
			}
		}
	}

	bool UpdateAudioSettings(size_t slotIdx, AudioUpdateSettings&& sets, AudioBank& audioBank)
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

	template <SomeMixType T>
	AudioStatus ExecuteAudioPlayCommand(size_t slotIdx, AudioPlayCommand command, AudioBank& audioBank)
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
			else
			{
				const auto fadeOut = (command == AudioPlayCommand::Halt) ? 0 : settings[0].fadeMs.out;

				AudioChannel::Stop<T>(channel, fadeOut);

				status[slotIdx] = AudioChannel::GetAudioStatus<T>(channel);
			}

			break;

		case AudioPlayCommand::Pause:
			if (IsActiveSlot(slotIdx))
			{
				AudioChannel::Pause<T>(channel);

				status[slotIdx] = AudioChannel::GetAudioStatus<T>(channel);
			}
			
			break;

		case AudioPlayCommand::Resume:
			if (IsActiveSlot(slotIdx))
			{
				AudioChannel::Resume<T>(channel);

				status[slotIdx] = AudioChannel::GetAudioStatus<T>(channel);
			}

			break;

		case AudioPlayCommand::Restart:
			if (IsActiveSlot(slotIdx))
			{
				settings[0].trackPosition = 0.0f;

				auto* audioPtr = audioBank.GetAudioPtr<T>(audioHandles[0]);

				AudioChannel::Restart<T>(audioPtr, channel, settings[0].loopCount, settings[0].fadeMs.in);

				status[slotIdx] = AudioChannel::GetAudioStatus<T>(channel);
			}

			break;

		case AudioPlayCommand::None:
			break;
		}

		return status[slotIdx];
	}

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

class AudioManager2
{
public:
	AudioManager2()
	{
		for (size_t i = 0; i < audioQueues_.size(); ++i)
		{
			audioQueues_[i].channel = static_cast<int>(i);
		}
	}

	Result<AudioInstanceID> StageAudio(const Handle<Audio>& handle, 
									   AudioChannelSettings&& sets,
									   uint8_t force, AudioBank& audioBank)
	{
		if (!audioBank.IsAudioValid(handle))
		{
			return MAKE_ERROR("Audio handle was invalid");
		}

		const auto audioType = audioBank.GetAudioInfo<&AudioInfo::audioType>(handle);
		assert(audioType);
		assert((*audioType == AudioType::Music || *audioType == AudioType::Sound));

		if (*audioType == AudioType::Music)
		{
			Mix_Music* musicPtr = audioBank.GetAudioPtr<Mix_Music>(handle);
			assert(musicPtr);

			auto instanceId = AudioInstanceID::Create();

			auto& musicQueue = audioQueues_[static_cast<size_t>(AudioQueue::kMusicChannelIndex)];

			TRY(musicQueue.StageAudio(musicPtr, handle, instanceId, std::move(sets), force), outcome);

			if (outcome.replacedInstance.IsValid())
			{
				queueIndexMap_.erase(outcome.replacedInstance);
			}

			queueIndexMap_.try_emplace(instanceId, QueueIndex{
				.channel = AudioQueue::kMusicChannelIndex, 
				.slot = outcome.assignedSlot
			});

			return instanceId;
		}
		else
		{
			Mix_Chunk* soundPtr = audioBank.GetAudioPtr<Mix_Chunk>(handle);
			assert(soundPtr);

			auto instanceId = AudioInstanceID::Create();

			auto& soundQueue = audioQueues_[soundChannelPriorityIdx_];

			TRY(soundQueue.StageAudio(soundPtr, handle, instanceId, std::move(sets), force), outcome);

			if (outcome.replacedInstance.IsValid())
			{
				queueIndexMap_.erase(outcome.replacedInstance);
			}

			queueIndexMap_.try_emplace(instanceId, QueueIndex{
				.channel = static_cast<int>(soundChannelPriorityIdx_), 
				.slot = outcome.assignedSlot
			});

			soundChannelPriorityIdx_ = 
				(soundChannelPriorityIdx_ + 1) % static_cast<size_t>(AudioQueue::kMusicChannelIndex);

			return instanceId;
		}
	}

	AudioStatus ExecuteAudioPlayCommand(const AudioInstanceID& instanceId, AudioPlayCommand command,
										AudioBank& audioBank)
	{
		auto it = queueIndexMap_.find(instanceId);
		if (it == queueIndexMap_.end())
		{
			return AudioStatus::Stopped;
		}

		const auto& queueIdx = it->second;
		if (!queueIdx.IsValid())
		{
			queueIndexMap_.erase(it);

			return AudioStatus::Stopped;
		}

		AudioStatus status;
		if (queueIdx.channel == AudioQueue::kMusicChannelIndex)
		{
			auto& musicQueue = audioQueues_[static_cast<size_t>(AudioQueue::kMusicChannelIndex)];

			status = musicQueue.ExecuteAudioPlayCommand<Mix_Music>(queueIdx.slot, command, audioBank);
		}
		else
		{
			auto& soundQueue = audioQueues_[static_cast<size_t>(queueIdx.channel)];

			status = soundQueue.ExecuteAudioPlayCommand<Mix_Chunk>(queueIdx.slot, command, audioBank);
		}

		if (status == AudioStatus::Stopped)
		{
			queueIndexMap_.erase(it);
		}

		return status;
	}

	bool UpdateAudioSettings(const AudioInstanceID& instanceId, AudioUpdateSettings&& updateSettings,
							 AudioBank& audioBank)
	{
		auto it = queueIndexMap_.find(instanceId);
		if (it == queueIndexMap_.end())
		{
			return false;
		}

		const auto& queueIdx = it->second;
		if (!queueIdx.IsValid())
		{
			queueIndexMap_.erase(it);

			return false;
		}

		auto& queue = audioQueues_[static_cast<size_t>(queueIdx.channel)];

		const bool success = queue.UpdateAudioSettings(queueIdx.slot, std::move(updateSettings), 
													   audioBank);
		if (!success)
		{
			queueIndexMap_.erase(it);
		}

		return success;
	}

	void UpdateAudioQueues(AudioBank& audioBank, float dt)
	{
		for (auto& queue : audioQueues_)
		{
			queue.UpdateSlots(queueIndexMap_, audioBank, dt);
		}
	}

	bool IsAudioInstanceValid(const AudioInstanceID& instanceId) const
	{
		auto it = queueIndexMap_.find(instanceId);

		return it != queueIndexMap_.end() && it->second.IsValid();
	}

	bool UpdateActiveAudioComponent(ActiveAudio& aa)
	{
		auto it = queueIndexMap_.find(aa.instanceId);

		if (it == queueIndexMap_.end() || !it->second.IsValid())
		{
			return false;
		}

		const auto& queueIdx = it->second;

		auto& audioQueue = audioQueues_[static_cast<size_t>(queueIdx.channel)];

		assert(aa.audioHandle == audioQueue.audioHandles[queueIdx.slot]);
		assert(aa.status != AudioStatus::Stopped); // should have been removed from queueIdxMap
		
		aa.settings = audioQueue.settings[queueIdx.slot];
		aa.status = audioQueue.status[queueIdx.slot];
		aa.onChannel = static_cast<size_t>(queueIdx.channel);

		return true;
	}

private:
	std::array<AudioQueue, AudioQueue::kMusicChannelIndex + 1> audioQueues_;
	size_t soundChannelPriorityIdx_ = 0;
	QueueIndexMap queueIndexMap_;
};


class AudioSystem2 : public System
{
public:
	void Update(float dt, AudioBank& audioBank)
	{
		HandleAudioUpdateRequests(audioBank);

		auto newAudioReqEs = ECS::GetAllEntitiesWith<NewAudioRequest>();

		PreprocessNewAudioRequests(newAudioReqEs, audioBank);

		audioManager_.UpdateAudioQueues(audioBank, dt);

		StageNewAudioRequests(newAudioReqEs, audioBank);

		UpdateActiveAudio();
	}


private:
	void HandleAudioUpdateRequests(AudioBank& audioBank)
	{
		auto es = ECS::GetAllEntitiesWith<AudioUpdateRequest>();

		for (auto& e : es)
		{
			auto& req = e.GetComponent<AudioUpdateRequest>();

			const bool updated = audioManager_.UpdateAudioSettings(
				req.instanceId,
				std::move(req.settings),
				audioBank
			);

			if (updated)
			{
				audioManager_.ExecuteAudioPlayCommand(req.instanceId, req.command, audioBank);
			}
			else
			{
				LOG_ERROR("AudioUpdateRequest referred to an invalid AudioInstanceID");
			}

			e.RemoveComponent<AudioUpdateRequest>();
		}
	}

	void PreprocessNewAudioRequests(std::vector<Entity>& newAudioReqEs, AudioBank& audioBank)
	{
		for (auto& e : newAudioReqEs)
		{
			assert(!e.HasComponent<AudioUpdateRequest>());
			assert(e.HasComponent<NewAudioRequest>());

			if (e.HasComponent<ActiveAudio>())
			{
				auto& aa = e.GetComponent<ActiveAudio>(GetEntityPassKey());

				const auto status = audioManager_.ExecuteAudioPlayCommand(
					aa.instanceId,
					AudioPlayCommand::Halt,
					audioBank
				);

				assert(status == AudioStatus::Stopped);

				e.RemoveComponent<ActiveAudio>(GetEntityPassKey());
			}
		}
	}

	void StageNewAudioRequests(std::vector<Entity>& newAudioReqEs, AudioBank& audioBank)
	{
		for (auto& e : newAudioReqEs)
		{
			assert(!e.HasComponent<AudioUpdateRequest>());
			assert(!e.HasComponent<ActiveAudio>());
			assert(e.HasComponent<NewAudioRequest>());

			auto& req = e.GetComponent<NewAudioRequest>();

			auto instanceResult = audioManager_.StageAudio(
				req.audioHandle,
				std::move(req.settings),
				req.force,
				audioBank
			);

			if (!instanceResult.Success())
			{
				LOG_ERROR(instanceResult.GetError().GetMessage());
			}
			else
			{
				e.AddComponent(ActiveAudio{
					.audioHandle = req.audioHandle,
					.instanceId = instanceResult.GetValue()
				}, GetEntityPassKey());
			}

			e.RemoveComponent<NewAudioRequest>();
		}
	}

	void UpdateActiveAudio()
	{
		auto es = ECS::GetAllEntitiesWith<ActiveAudio>();

		for (auto& e : es)
		{
			assert(!e.HasComponent<NewAudioRequest>());
			assert(!e.HasComponent<AudioUpdateRequest>());

			auto& aa = e.GetComponent<ActiveAudio>(GetEntityPassKey());

			const bool success = audioManager_.UpdateActiveAudioComponent(aa);
			if (!success)
			{
				e.RemoveComponent<ActiveAudio>(GetEntityPassKey());
			}
		}
	}

	AudioManager2 audioManager_;
};