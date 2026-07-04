#include "AudioSystem.h"
#include "../ecs/Ecs.h"
#include <cassert>

//// TODO: Remove instance id audio update request component
inline void AudioSystem::HandleAudioUpdateRequests()
{
	auto entities = ECS::GetAllEntitiesWith<AudioUpdateRequest>();

	for (auto& entity : entities)
	{
		auto consistencyResult = ResolveUpdateRequestInstanceId(entity);
		if (!consistencyResult.Success())
		{
			LOG_ERROR(consistencyResult.GetError());
			entity.RemoveComponent<AudioUpdateRequest>();

			continue;
		}

		auto& updateRequest = entity.GetComponent<AudioUpdateRequest>();

		audioManager_.UpdateAudioSettings(updateRequest.instanceId,
										  std::move(updateRequest.settings));

		AudioStatus status = audioManager_.ExecuteAudioCommand(
			updateRequest.instanceId, updateRequest.command
		);

		if (status == AudioStatus::Stopped)
		{
			entity.RemoveComponent<ActiveAudio>(GetEntityPassKey());
		}

		entity.RemoveComponent<AudioUpdateRequest>();
	}
}

void AudioSystem::HandleNewAudioRequests()
{
	auto entities = ECS::GetAllEntitiesWith<NewAudioRequest>();

	for (auto& entity : entities)
	{
		auto& newRequest = entity.GetComponent<NewAudioRequest>();

		auto audioTypeOp = 
			audioBank_.GetAudioInfo<&AudioInfo::audioType>(newRequest.audioHandle);
		if (!audioTypeOp.has_value())
		{
			LOG_ERROR("Failed to get audio type for handle");

			entity.RemoveComponent<NewAudioRequest>();

			continue;
		}

		const auto audioType = *audioTypeOp;
		switch (audioType)
		{
		case AudioType::Sound:
		{
			auto instanceResult = audioBank_.GetSoundInstanceResouce(newRequest.audioHandle);
			if (!instanceResult.Success())
			{
				LOG_ERROR(instanceResult.GetError());

				break;
			}

			assert(instanceResult.GetValue().audioPtr);

			const AudioInstanceID instanceId = instanceResult.GetValue().id;
			assert(instanceId.IsValid());

			SoundStageSlot stageSlot{
				.instance = std::move(instanceResult.GetValue()),
				.settings = std::move(newRequest.settings),
				.force = newRequest.force
			};

			auto [channelIdx, status] = audioManager_.StageAudio(std::move(stageSlot));
			if (channelIdx == AudioManager::kInvalidChannelIndex)
			{
				LOG_WARNING("Audio could not be staged");

				break;
			}

			assert(status == AudioStatus::Staged);

			// successfully staged. check if this entity had active audio and stop/unstage it
			if (entity.HasComponent<ActiveAudio>())
			{
				auto& oldActiveAudio = entity.GetComponent<ActiveAudio>();

				AudioStatus stopResultStatus = 
					audioManager_.ExecuteAudioCommand(oldActiveAudio.instanceId, 
													  AudioPlayCommand::Stop);

				assert((stopResultStatus == AudioStatus::Stopped && 
					    oldActiveAudio.status == AudioStatus::Staged) ||
				       (stopResultStatus == AudioStatus::Stopping &&
						oldActiveAudio.status != AudioStatus::Staged));
			}

			// add active audio either adds new one or overwrites old one
			// (it's okay if the old active audio is stopping, 
			//  this only affects what the entity sees)
			auto& activeAudio = entity.AddComponent<ActiveAudio>(GetEntityPassKey());
			activeAudio.audioHandle = newRequest.audioHandle;
			activeAudio.instanceId = instanceId;

			break;
		}
		case AudioType::Music:
		{
			auto instanceResult = audioBank_.GetMusicInstanceResource(newRequest.audioHandle);
			if (!instanceResult.Success())
			{
				LOG_ERROR(instanceResult.GetError());

				break;
			}

			assert(instanceResult.GetValue().audioPtr);

			const AudioInstanceID instanceId = instanceResult.GetValue().id;
			assert(instanceId.IsValid());

			MusicStageSlot stageSlot{
				.instance = std::move(instanceResult.GetValue()),
				.settings = std::move(newRequest.settings),
				.force = newRequest.force
			};

			auto [channelIdx, status] = audioManager_.StageAudio(std::move(stageSlot));
			if (channelIdx == AudioManager::kInvalidChannelIndex)
			{
				LOG_WARNING("Audio could not be staged");

				break;
			}

			assert(status == AudioStatus::Staged);

			// successfully staged. check if this entity had active audio and stop/unstage it
			if (entity.HasComponent<ActiveAudio>())
			{
				auto& oldActiveAudio = entity.GetComponent<ActiveAudio>();

				AudioStatus stopResultStatus =
					audioManager_.ExecuteAudioCommand(oldActiveAudio.instanceId,
													  AudioPlayCommand::Stop);

				assert((stopResultStatus == AudioStatus::Stopped &&
					    oldActiveAudio.status == AudioStatus::Staged) ||
					   (stopResultStatus == AudioStatus::Stopping &&
						oldActiveAudio.status != AudioStatus::Staged));
			}

			// add active audio either adds new one or overwrites old one
			// (it's okay if the old active audio is stopping, 
			//  this only affects what the entity sees)
			auto& activeAudio = entity.AddComponent<ActiveAudio>(GetEntityPassKey());
			activeAudio.audioHandle = newRequest.audioHandle;
			activeAudio.instanceId = instanceId;

			break;
		}
		case AudioType::Unknown: default:
		{
			LOG_ERROR("Audio could not be staged: Unrecognized AudioType from handle");
			break;
		}
		}

		entity.RemoveComponent<NewAudioRequest>();
	}
}

void AudioSystem::UpdateActiveAudioComponents()
{
	auto entities = ECS::GetAllEntitiesWith<ActiveAudio>();

	for (auto& entity : entities)
	{
		// updates should have been handled and removed already
		assert(!entity.HasComponent<NewAudioRequest>());
		assert(!entity.HasComponent<AudioUpdateRequest>());

		auto& activeAudio = entity.GetComponent<ActiveAudio>(GetEntityPassKey());
		if (!audioManager_.AudioInstanceValid(activeAudio.instanceId))
		{
			// audio was stopped
			entity.RemoveComponent<ActiveAudio>(GetEntityPassKey());
			continue;
		}

		auto [instanceChannelIdx, instanceStatus] =
			audioManager_.GetAudioInstanceChannelAndStatus(activeAudio.instanceId);
		assert(instanceChannelIdx <= AudioManager::kMusicChannelIndex);
		
		if (instanceStatus == AudioStatus::Stopped)
		{
			// TODO: Decide if audio manager should auto-remove stopped instances or not
			entity.RemoveComponent<ActiveAudio>(GetEntityPassKey());
			continue;
		}

		activeAudio.onChannel = instanceChannelIdx;
		activeAudio.status = instanceStatus;
		activeAudio.settings = audioManager_.GetInstanceAudioSettings(activeAudio.instanceId);
	}
}

Result<Void> AudioSystem::ResolveUpdateRequestInstanceId(Entity& entity)
{
	assert(entity.IsValid());
	assert(entity.HasComponent<AudioUpdateRequest>());

	if (!entity.HasComponent<ActiveAudio>())
	{
		return MAKE_ERROR("Entity had audio update request but no active audio");
	}

	auto& activeAudio = entity.GetComponent<ActiveAudio>();
	auto& updateRequest = entity.GetComponent<AudioUpdateRequest>();

	if (!updateRequest.instanceId.IsValid())
	{
		updateRequest.instanceId = activeAudio.instanceId;
	}

	//if (activeAudio.instanceId != updateRequest.instanceId)
	//{
	//	return MAKE_ERROR("Active audio's instance id differs from "
	//		"update request's instance id");
	//}

	return Void{};
}


void AudioSystem::Update(float dt)
{
	HandleAudioUpdateRequests();
	HandleNewAudioRequests();

	audioManager_.UpdateChannels(dt);

	UpdateActiveAudioComponents();
}

void AudioSystem::EntityDestroyed(Entity& entity)
{
	assert(entity.IsValid());

	if (!entity.HasComponent<ActiveAudio>())
	{
		return;
	}

	AudioStatus status = audioManager_.ExecuteAudioCommand(
		entity.GetComponent<ActiveAudio>().instanceId, AudioPlayCommand::Stop
	);

	assert(status == AudioStatus::Stopping || status == AudioStatus::Stopped);
}

void AudioSystem::CleanupForNewAudioBank()
{
	auto entities = ECS::GetAllEntitiesWithAny<
		NewAudioRequest, AudioUpdateRequest, ActiveAudio>();

	for (auto& entity : entities)
	{
		entity.RemoveComponent<NewAudioRequest>();
		entity.RemoveComponent<AudioUpdateRequest>();
		entity.RemoveComponent<ActiveAudio>(GetEntityPassKey());
	}

	audioManager_.ClearChannels();
	audioManager_.ClearStage();
}


void AudioSystem::SetAudioBank(AudioBank&& bank)
{
	CleanupForNewAudioBank();

	audioBank_ = std::move(bank);
}

AudioBank&& AudioSystem::SwapAudioBank(AudioBank&& newBank)
{
	CleanupForNewAudioBank();

	std::swap(audioBank_, newBank);

	return std::move(newBank);
}
