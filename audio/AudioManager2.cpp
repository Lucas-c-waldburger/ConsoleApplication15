#include "AudioManager2.h"

AudioManager2::AudioManager2()
{
	for (size_t i = 0; i < audioChannels_.size(); ++i)
	{
		audioChannels_[i] = AudioChannel{ i };
	}
}

AudioInstanceID AudioManager2::StageAudio(const NewAudioRequest& req, AudioBank& audioBank)
{
	assert(audioBank.IsAudioValid(req.audioHandle));

	const auto audioType = audioBank.GetAudioInfo<&AudioInfo::audioType>(req.audioHandle);
	assert(audioType);
	assert((*audioType == AudioType::Music || *audioType == AudioType::Sound));

	size_t channelIdx = std::numeric_limits<size_t>::max();
	if (*audioType == AudioType::Music)
	{
		channelIdx = static_cast<size_t>(AudioQueue::kMusicChannelIndex);
	}
	else
	{
		for (size_t i = 0; i < MIX_CHANNELS; ++i)
		{
			// loop through all sound channels until find an open one or all full
			if (!audioChannels_[soundChannelPriorityIdx_].IsOccupied())
			{
				break;
			}

			soundChannelPriorityIdx_ = (soundChannelPriorityIdx_ + 1) % MIX_CHANNELS;
		}

		channelIdx = soundChannelPriorityIdx_;

		soundChannelPriorityIdx_ = (soundChannelPriorityIdx_ + 1) % MIX_CHANNELS;
	}

	auto outcome = audioChannels_[channelIdx].StageAudio(req, audioInstanceToChannelMap_, audioBank);
	if (!outcome.assignedInstance.IsValid())
	{
		assert(!outcome.replacedInstance.IsValid());

		return AudioInstanceID{};
	}

	if (outcome.replacedInstance.IsValid())
	{
		audioInstanceToChannelMap_.erase(outcome.replacedInstance);
	}

	auto [_, inserted] =
		audioInstanceToChannelMap_.try_emplace(outcome.assignedInstance, channelIdx);
	assert(inserted);

	return outcome.assignedInstance;
}

AudioStatus AudioManager2::ExecuteAudioPlayCommand(const AudioInstanceID& instanceId,
												   AudioPlayCommand command, AudioBank& audioBank)
{
	auto it = audioInstanceToChannelMap_.find(instanceId);
	if (it == audioInstanceToChannelMap_.end())
	{
		return AudioStatus::Stopped;
	}

	const size_t channelIdx = it->second;
	if (channelIdx >= audioChannels_.size())
	{
		audioInstanceToChannelMap_.erase(it);

		return AudioStatus::Stopped;
	}

	const auto status = audioChannels_[channelIdx].ExecuteAudioPlayCommand(command, audioBank);
	if ((status == AudioStatus::Stopped || status == AudioChannel::kChannelEmptyStatus))
	{
		audioInstanceToChannelMap_.erase(it);
	}

	return status;
}

bool AudioManager2::ApplyAudioUpdateSettings(const AudioInstanceID& instanceId, 
											 AudioUpdateSettings&& updateSettings, AudioBank& audioBank)
{
	auto it = audioInstanceToChannelMap_.find(instanceId);
	if (it == audioInstanceToChannelMap_.end())
	{
		return false;
	}

	const size_t channelIdx = it->second;
	if (channelIdx >= audioChannels_.size())
	{
		audioInstanceToChannelMap_.erase(it);

		return false;
	}

	const bool success = audioChannels_[channelIdx].ApplyUpdateSettingsToActiveAudio(
		std::move(updateSettings), audioInstanceToChannelMap_, audioBank);
	if (!success)
	{
		audioInstanceToChannelMap_.erase(it);
	}

	return success;
}

void AudioManager2::UpdateAudioChannels(AudioBank& audioBank, float dt)
{
	for (auto& channel : audioChannels_)
	{
		const auto status = channel.Update(audioInstanceToChannelMap_, audioBank, dt);

		if (status == AudioStatus::Stopped)
		{
			audioInstanceToChannelMap_.erase(channel.GetActiveAudio().instanceId);

			channel.ClearActiveAudio();
		}
	}
}

bool AudioManager2::IsAudioInstanceValid(const AudioInstanceID& instanceId) const
{
	auto it = audioInstanceToChannelMap_.find(instanceId);

	return it != audioInstanceToChannelMap_.end() && it->second < audioChannels_.size();
}

bool AudioManager2::UpdateActiveAudioComponent(ActiveAudio& aa)
{
	auto it = audioInstanceToChannelMap_.find(aa.instanceId);
	if (it == audioInstanceToChannelMap_.end())
	{
		return false;
	}

	const size_t channelIdx = it->second;
	if (channelIdx >= audioChannels_.size())
	{
		audioInstanceToChannelMap_.erase(it);

		return false;
	}

	if (!audioChannels_[channelIdx].IsOccupied())
	{
		audioInstanceToChannelMap_.erase(it);

		return false;
	}

	const auto& channelActiveAudio = audioChannels_[channelIdx].GetActiveAudio();

	assert(aa.audioHandle == channelActiveAudio.audioHandle);
	assert(aa.instanceId == channelActiveAudio.instanceId);
	assert(aa.onChannel == channelActiveAudio.onChannel);

	aa = channelActiveAudio;

	return true;
}

const ActiveAudio& AudioManager2::GetActiveAudioForInstance(const AudioInstanceID& instanceId) const
{
	auto it = audioInstanceToChannelMap_.find(instanceId);
	if (it == audioInstanceToChannelMap_.end() || it->second >= audioChannels_.size())
	{
		return Null<ActiveAudio>();
	}

	return audioChannels_[it->second].GetActiveAudio();
}

void AudioManager2::PauseAll()
{
	for (auto& channel : audioChannels_)
	{
		channel.PauseDirect();
	}
}

void AudioManager2::ResumeAll()
{
	for (auto& channel : audioChannels_)
	{
		channel.ResumeDirect();
	}
}

void AudioManager2::HaltInstance(const AudioInstanceID& instanceId)
{
	auto it = audioInstanceToChannelMap_.find(instanceId);
	if (it == audioInstanceToChannelMap_.end())
	{
		return;
	}

	const size_t channelIdx = it->second;
	if (channelIdx >= audioChannels_.size())
	{
		audioInstanceToChannelMap_.erase(it);

		return;
	}

	audioChannels_[channelIdx].HaltDirect();

	audioInstanceToChannelMap_.erase(it);
}
