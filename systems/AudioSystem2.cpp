#include "AudioSystem2.h"
#include <ranges>
#include "../ecs/Ecs.h"

AudioSystem2::AudioSystem2()
{
	ObserveEntityDestroyed();
}

void AudioSystem2::Update(float dt, AudioBank& audioBank)
{
	if (!IsPaused())
	{
		HandleAudioUpdateRequests(audioBank);

		auto newAudioReqEs = ECS::GetAllEntitiesWith<NewAudioRequest>();

		PreprocessNewAudioRequests(newAudioReqEs, audioBank);

		audioManager_.UpdateAudioChannels(audioBank, dt);

		StageNewAudioRequests(newAudioReqEs, audioBank, dt);

		UpdateActiveAudio();
	}
	else
	{
		audioManager_.UpdateAudioChannels(audioBank, dt);

		UpdateActiveAudio();
	}
}  

void AudioSystem2::HandleAudioUpdateRequests(AudioBank& audioBank)
{
	auto es = ECS::GetAllEntitiesWith<AudioUpdateRequest>();

	for (auto& e : es)
	{
		auto& req = e.GetComponent<AudioUpdateRequest>();

		if (!req.instanceId.IsValid() && e.HasComponent<ActiveAudio>())
		{
			// default to the entity-owned audio
			req.instanceId = e.GetComponent<ActiveAudio>().instanceId;
		}

		const bool updated = audioManager_.ApplyAudioUpdateSettings(
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

void AudioSystem2::PreprocessNewAudioRequests(std::vector<Entity>& newAudioReqEs, AudioBank& audioBank)
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

			assert((status == AudioStatus::Stopped || 
				    status == AudioChannel::kChannelEmptyStatus));

			e.RemoveComponent<ActiveAudio>(GetEntityPassKey());
		}
	}
}

void AudioSystem2::StageNewAudioRequests(std::vector<Entity>& newAudioReqEs, AudioBank& audioBank, float dt)
{
	std::ranges::sort(newAudioReqEs, [](const auto& e1, const auto& e2) {
		return e1.GetComponent<NewAudioRequest>().timeInQueue >
			   e2.GetComponent<NewAudioRequest>().timeInQueue;
	});

	for (auto& e : newAudioReqEs)
	{
		assert(!e.HasComponent<AudioUpdateRequest>());
		assert(!e.HasComponent<ActiveAudio>());
		assert(e.HasComponent<NewAudioRequest>());

		auto& req = e.GetComponent<NewAudioRequest>();

		if (!audioBank.IsAudioValid(req.audioHandle))
		{
			LOG_ERROR("Could not stage audio - audio handle was invalid");

			e.RemoveComponent<NewAudioRequest>();
		}

		const auto instanceId = audioManager_.StageAudio(req, audioBank);
		if (!instanceId.IsValid())
		{
			LOG_DEBUG("Could not stage audio yet...All channels full");

			req.timeInQueue += dt;

			continue;
		}

		auto aa = audioManager_.GetActiveAudioForInstance(instanceId);
		assert(aa.audioHandle == req.audioHandle);
		assert(aa.instanceId == instanceId);

		e.AddComponent(std::move(aa), GetEntityPassKey());

		e.RemoveComponent<NewAudioRequest>();
	}
}

void AudioSystem2::UpdateActiveAudio()
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

void AudioSystem2::OnEntityDestroyed(Entity entity)
{
	assert(entity.IsValid());

	if (!entity.HasComponent<ActiveAudio>())
	{
		return;
	}

	audioManager_.HaltInstance(entity.GetComponent<ActiveAudio>().instanceId);
}

void AudioSystem2::SetPausedImpl(bool doPause)
{
	if (doPause)
	{
		audioManager_.PauseAll();
	}
	else
	{
		audioManager_.ResumeAll();
	}
}