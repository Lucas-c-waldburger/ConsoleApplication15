#include "AudioManager.h"


namespace {

bool IsAudioSpatialDataEmpty(const AudioSpatialData& spatialData)
{
    return !(spatialData.angle.has_value() ||
             spatialData.distance.has_value() ||
             spatialData.panning.has_value());
}

template <typename T>
constexpr bool ValidStageSlot(const AudioStageSlot<T>& slot)
{
    return slot.instance.audioPtr != nullptr && slot.instance.id.IsValid();
}

template <typename T>
constexpr bool EmptyStageSlot(const AudioStageSlot<T>& slot)
{
    return !slot.instance.audioPtr;
}

template <typename Ch>
bool ChannelAvailable(const Ch& channel)
{
    return !(channel.IsPlaying() || channel.IsPaused());
}

template <typename Ch>
bool ShouldStopChannel(const Ch& channel, uint8_t force)
{
    return !channel.IsStopping() &&
        (force & (AudioForcing::ForceChannelGraceful |
            AudioForcing::ForceChannelHalt)) != 0;
}
template <typename T>
constexpr bool StageSlotForced(const AudioStageSlot<T>& slot)
{
    return (slot.force & AudioForcing::ForceStage) != 0;
}



} // unnamed

AudioChannels::AudioChannels()
{
    for (size_t i = 0; i < soundChannels.size(); i++)
    {
        soundChannels[i].first = SoundChannel{ i };
    }
}

void AudioManager::ClearChannels()
{
    auto& musicChannel = channels_.musicChannel.first;
    musicChannel.Stop();
    musicChannel.SetMusicInstance({});

    for (auto& [soundChannel, _] : channels_.soundChannels)
    {
        soundChannel.Stop();
        soundChannel.SetSoundInstance({});
    }
}

void AudioManager::ClearStage()
{
    auto& [musicWaiting, musicOnDeck] = stage_.stagedMusic;

    musicWaiting.instance = {};
    musicOnDeck.instance = {};

    for (auto& [soundWaiting, soundOnDeck] : stage_.stagedSounds)
    {
        soundWaiting.instance = {};
        soundOnDeck.instance = {};
    }

    stage_.fairSoundForceIdx = 0;
}

void AudioManager::UpdateChannels()
{
    UpdateMusicChannel();
    UpdateSoundChannels();
}

AudioManager::InstanceChannelAndStatus 
AudioManager::GetAudioInstanceChannelAndStatus(const AudioInstanceID& instanceId) const
{
    auto it = instanceLog_.find(instanceId);

    return (it != instanceLog_.end()) ? it->second : kInvalidInstanceChannelAndStatus;
}

AudioSettings AudioManager::GetInstanceAudioSettings(const AudioInstanceID& instanceId) const
{
    auto it = instanceLog_.find(instanceId);
    if (it == instanceLog_.end())
    {
        LOG_ERROR("Could not retrieve instance's audio settings: Instance Id not found in log");
        return {};
    }

    const auto [channelIdx, status] = it->second;
    assert(channelIdx <= kMusicChannelIndex);

    if (status == AudioStatus::Staged)
    {
        if (channelIdx == kMusicChannelIndex)
        {
            auto& [musicWaiting, musicOnDeck] = stage_.stagedMusic;

            auto& matchingSlot = (musicWaiting.instance.id == instanceId)
                ? musicWaiting : musicOnDeck;

            assert(matchingSlot.instance.id == instanceId);

            return matchingSlot.settings;
        }
        else
        {
            auto& [soundWaiting, soundOnDeck] = stage_.stagedSounds[channelIdx];

            auto& matchingSlot = (soundWaiting.instance.id == instanceId)
                ? soundWaiting : soundOnDeck;

            assert(matchingSlot.instance.id == instanceId);

            return matchingSlot.settings;
        }
    }

    // not staged, should be in a channel
    if (channelIdx == kMusicChannelIndex)
    {
        auto& [musicChannel, musicSettings] = channels_.musicChannel;

        assert(musicChannel.GetMusicInstance().id == instanceId);

        return musicSettings;
    }
    else
    {
        auto& [soundChannel, soundSettings] = channels_.soundChannels[channelIdx];

        assert(soundChannel.GetActiveSoundInstance().id == instanceId);

        return soundSettings;
    }
}

void AudioManager::UpdateAudioSettings(const AudioInstanceID& instanceId,
                                       AudioSettings&& newSettings)
{
    auto it = instanceLog_.find(instanceId);
    if (it == instanceLog_.end())
    {
        LOG_ERROR("Could not apply audio settings: Instance Id not found in log");
        return;
    }

    const auto [channelIdx, status] = it->second;
    assert(channelIdx <= kMusicChannelIndex);

    if (status == AudioStatus::Staged)
    {
        if (channelIdx == kMusicChannelIndex)
        {
            auto& [musicWaiting, musicOnDeck] = stage_.stagedMusic;

            auto& matchingSlot = (musicWaiting.instance.id == instanceId)
                ? musicWaiting : musicOnDeck;

            assert(matchingSlot.instance.id == instanceId);

            matchingSlot.settings = std::move(newSettings);

            return;
        }
        else
        {
            auto& [soundWaiting, soundOnDeck] = stage_.stagedSounds[channelIdx];

            auto& matchingSlot = (soundWaiting.instance.id == instanceId)
                ? soundWaiting : soundOnDeck;

            assert(matchingSlot.instance.id == instanceId);

            matchingSlot.settings = std::move(newSettings);

            return;
        }
    }

    // not staged, should be in a channel
    if (channelIdx == kMusicChannelIndex)
    {
        auto& [musicChannel, oldMusicSettings] = channels_.musicChannel;
        assert(musicChannel.GetMusicInstance().id == instanceId);

        if (oldMusicSettings.baseVolume != newSettings.baseVolume)
        {
            musicChannel.SetVolume(newSettings.baseVolume);
        }

        oldMusicSettings = std::move(newSettings);;
    }
    else
    {
        auto& [soundChannel, oldSoundSettings] = channels_.soundChannels[channelIdx];
        assert(soundChannel.GetActiveSoundInstance().id == instanceId);

        if (oldSoundSettings.baseVolume != newSettings.baseVolume)
        {
            soundChannel.SetVolume(newSettings.baseVolume);
        }

        oldSoundSettings = std::move(newSettings);
    }
}

AudioStatus AudioManager::ExecuteAudioCommand(const AudioInstanceID& instanceId, 
                                              AudioPlayCommand command)
{
    auto it = instanceLog_.find(instanceId);
    if (it == instanceLog_.end())
    {
        LOG_ERROR("Could not execute audio command: Instance Id not found in log");
        return AudioStatus::Stopped;
    }

    auto [channelIdx, status] = it->second;
    assert(channelIdx <= kMusicChannelIndex);

    if (command == AudioPlayCommand::None)
    {
        return status;
    }

    if (status == AudioStatus::Staged)
    {
        // if staged, only care about Stop (it means "unstage" in this context)
        if (command == AudioPlayCommand::Stop)
        {
            if (channelIdx == kMusicChannelIndex)
            {
                auto& [musicWaiting, musicOnDeck] = stage_.stagedMusic;

                auto& matchingSlot = (musicWaiting.instance.id == instanceId)
                    ? musicWaiting : musicOnDeck;

                assert(matchingSlot.instance.id == instanceId);

                matchingSlot.instance = {};
                instanceLog_.erase(instanceId);
            }
            else
            {
                auto& [soundWaiting, soundOnDeck] = stage_.stagedSounds[channelIdx];

                auto& matchingSlot = (soundWaiting.instance.id == instanceId)
                    ? soundWaiting : soundOnDeck;

                assert(matchingSlot.instance.id == instanceId);

                matchingSlot.instance = {};
                instanceLog_.erase(instanceId);
            }
        }

        return AudioStatus::Stopped;
    }

    // audio instance is on a channel
    switch (command)
    {
    case AudioPlayCommand::Pause:
        (channelIdx == kMusicChannelIndex) 
            ? channels_.musicChannel.first.Pause()
            : channels_.soundChannels[channelIdx].first.Pause(); 

        status = AudioStatus::Paused;
        return status;

    case AudioPlayCommand::Resume:
        (channelIdx == kMusicChannelIndex)
            ? channels_.musicChannel.first.Resume()
            : channels_.soundChannels[channelIdx].first.Resume();

        status = AudioStatus::Playing;
        return status;

    case AudioPlayCommand::Restart:
        (channelIdx == kMusicChannelIndex)
            ? channels_.musicChannel.first.Play(
                channels_.musicChannel.second.loopCount,
                channels_.musicChannel.second.fadeMs.out)
            : channels_.soundChannels[channelIdx].first.Play(
                channels_.soundChannels[channelIdx].second.loopCount,
                channels_.soundChannels[channelIdx].second.fadeMs.out);

        status = AudioStatus::Playing;
        return status;

    case AudioPlayCommand::Stop:
    {
        (channelIdx == kMusicChannelIndex)
            ? channels_.musicChannel.first.Stop(
                channels_.musicChannel.second.fadeMs.out)
            : channels_.soundChannels[channelIdx].first.Stop(
                channels_.soundChannels[channelIdx].second.fadeMs.out);

        const auto& channelSettings = 
            (channelIdx == kMusicChannelIndex)
                ? channels_.musicChannel.second
                : channels_.soundChannels[channelIdx].second;

        const bool willStopImmediately = channelSettings.fadeMs.out <= 0;
        if (willStopImmediately)
        {
            instanceLog_.erase(it);
            return AudioStatus::Stopped;
        }
        else
        {
            status = AudioStatus::Stopping;
            return status;
        }
    }
    default:
        assert(false);
        return AudioStatus::Stopped;
    }
}

void AudioManager::SetSoundInstanceSpatialData(const AudioInstanceID& instanceId,
                                               const AudioSpatialData& spatialData)
{
    if (IsAudioSpatialDataEmpty(spatialData))
    {
        return;
    }

    auto it = instanceLog_.find(instanceId);
    if (it == instanceLog_.end())
    {
        LOG_ERROR("Could not set spatial data: Instance Id not found in log");
        return;
    }

    auto [channelIdx, status] = it->second;
    if (channelIdx >= kMusicChannelIndex)
    {
        LOG_ERROR("Could not set spatial data: Instance is not a sound");
        return;
    }

    if (status == AudioStatus::Staged || status == AudioStatus::Stopped)
    {
        LOG_ERROR("Could not set spatial data: Instance is not loaded on a channel");
        return;
    }

    auto& soundChannel = channels_.soundChannels[channelIdx].first;
    assert(soundChannel.GetActiveSoundInstance().id == instanceId);

    if (spatialData.angle.has_value())
    {
        uint8_t distance = spatialData.distance.value_or(0);

        soundChannel.SetPosition(*spatialData.angle, distance);

        return;
    }
    if (spatialData.distance.has_value())
    {
        soundChannel.SetDistance(*spatialData.distance);
    }
    if (spatialData.panning.has_value())
    {
        soundChannel.SetPanning(spatialData.panning->left,
                                spatialData.panning->right);
    }
}

bool AudioManager::AudioInstanceValid(const AudioInstanceID& instanceId) const
{
    auto it = instanceLog_.find(instanceId);
    if (it == instanceLog_.end())
    {
        return false;
    }

    auto& [channelIdx, status] = it->second;

    return channelIdx <= kMusicChannelIndex && status != AudioStatus::Stopped;
}    

AudioManager::InstanceChannelAndStatus
AudioManager::StageMusic(MusicStageSlot stageSlot)
{
    if (instanceLog_.contains(stageSlot.instance.id))
    {
        LOG_ERROR("Music not staged: Duplicate instance id in log");

        return kInvalidInstanceChannelAndStatus;
    }

    auto& [musicWaiting, musicOnDeck] = stage_.stagedMusic;

    if (EmptyStageSlot(musicWaiting))
    {
        auto logEntry = std::make_pair(kMusicChannelIndex, AudioStatus::Staged);
        instanceLog_[stageSlot.instance.id] = logEntry;
        musicWaiting = std::move(stageSlot);

        return logEntry;
    }
    if (EmptyStageSlot(musicOnDeck) || StageSlotForced(stageSlot))
    {
        if (ValidStageSlot(musicOnDeck))
        {
            assert(instanceLog_.contains(musicOnDeck.instance.id));
            instanceLog_.erase(musicOnDeck.instance.id);
        }

        auto logEntry = std::make_pair(kMusicChannelIndex, AudioStatus::Staged);
        instanceLog_[stageSlot.instance.id] = logEntry;
        musicOnDeck = std::move(stageSlot);

        return logEntry;
    }

    LOG_WARNING("Music could not be staged, no open slot");

    return kInvalidInstanceChannelAndStatus;
}

AudioManager::InstanceChannelAndStatus
AudioManager::StageSound(SoundStageSlot stageSlot)
{
    if (instanceLog_.contains(stageSlot.instance.id))
    {
        LOG_ERROR("Music not staged: Duplicate instance id in log");

        return kInvalidInstanceChannelAndStatus;
    }

    for (size_t i = 0; i < stage_.stagedSounds.size(); i++)
    {
        auto& [soundWaiting, soundOnDeck] = stage_.stagedSounds[i];

        if (EmptyStageSlot(soundWaiting))
        {
            auto logEntry = std::make_pair(i, AudioStatus::Staged);
            instanceLog_[stageSlot.instance.id] = logEntry;
            soundWaiting = std::move(stageSlot);

            return logEntry;
        }
        if (EmptyStageSlot(soundOnDeck))
        {
            auto logEntry = std::make_pair(i, AudioStatus::Staged);
            instanceLog_[stageSlot.instance.id] = logEntry;
            soundOnDeck = std::move(stageSlot);

            return logEntry;
        }
    }

    // no available channels, if forcing, overwrite equitably
    if (StageSlotForced(stageSlot))
    {
        size_t channelIdx = stage_.fairSoundForceIdx;
        auto& soundOnDeck = stage_.stagedSounds[channelIdx].second;

        assert(instanceLog_.contains(soundOnDeck.instance.id));
        instanceLog_.erase(soundOnDeck.instance.id);

        auto logEntry = std::make_pair(channelIdx, AudioStatus::Staged);
        instanceLog_[stageSlot.instance.id] = logEntry;
        soundOnDeck = std::move(stageSlot);

        stage_.fairSoundForceIdx =
            (channelIdx + 1) % stage_.stagedSounds.size();

        return logEntry;
    }

    LOG_WARNING("Sound could not be staged, no open slot");

    return kInvalidInstanceChannelAndStatus;
}

void AudioManager::UpdateMusicChannel()
{
    auto& [musicWaiting, musicOnDeck] = stage_.stagedMusic;
    auto& [musicChannel, musicChannelSettings] = channels_.musicChannel;

    if (EmptyStageSlot(musicWaiting))
    {
        if (EmptyStageSlot(musicOnDeck))
        {
            return;
        }
        std::swap(musicWaiting, musicOnDeck);
    }
    if (!ChannelAvailable(musicChannel))
    {
        if (ShouldStopChannel(musicChannel, musicWaiting.force))
        {
            auto it = instanceLog_.find(musicChannel.GetMusicInstance().id);
            assert(it != instanceLog_.end());

            auto& [instanceChannel, instanceStatus] = it->second;
            assert(instanceChannel == kMusicChannelIndex);
            assert(instanceStatus != AudioStatus::Staged);

            const bool willStopImmediately =
                (musicWaiting.force & AudioForcing::ForceChannelHalt) != 0 ||
                musicChannelSettings.fadeMs.out <= 0;

            if (willStopImmediately)
            {
                instanceLog_.erase(it);
            }
            else
            {
                instanceStatus = AudioStatus::Stopping;
            }
                  
            musicChannel.Stop(musicChannelSettings.fadeMs.out);
        }
    }
    if (ChannelAvailable(musicChannel)) // now put musicWaiting on channel
    {
        auto it = instanceLog_.find(musicWaiting.instance.id);
        assert(it != instanceLog_.end());

        auto& musicWaitingStatus = it->second.second;
        musicWaitingStatus = AudioStatus::Playing;

        musicChannel.SetMusicInstance(musicWaiting.instance);
        musicChannelSettings = std::move(musicWaiting.settings);
        musicWaiting.instance = {};
       
        musicChannel.Play(musicChannelSettings.loopCount,
            musicChannelSettings.fadeMs.in);

        if (ValidStageSlot(musicOnDeck))
        {
            std::swap(musicWaiting, musicOnDeck);
        }
    }
}

void AudioManager::UpdateSoundChannels()
{
    for (size_t i = 0; i < MIX_CHANNELS; i++)
    {
        auto& [soundWaiting, soundOnDeck] = stage_.stagedSounds[i];
        auto& [soundChannel, soundChannelSettings] = channels_.soundChannels[i];

        if (EmptyStageSlot(soundWaiting))
        {
            if (EmptyStageSlot(soundOnDeck))
            {
                continue;
            }
            std::swap(soundWaiting, soundOnDeck);
        }

        if (!ChannelAvailable(soundChannel))
        {
            if (ShouldStopChannel(soundChannel, soundOnDeck.force))
            {
                auto it = instanceLog_.find(soundChannel.GetActiveSoundInstance().id);
                assert(it != instanceLog_.end());

                auto& [instanceChannel, instanceStatus] = it->second;
                assert(instanceChannel == soundChannel.GetChannelIndex());
                assert(instanceStatus != AudioStatus::Staged);

                const bool willStopImmediately =
                    (soundWaiting.force & AudioForcing::ForceChannelHalt) != 0 ||
                    soundChannelSettings.fadeMs.out <= 0;

                if (willStopImmediately)
                {
                    instanceLog_.erase(it);
                }
                else
                {
                    instanceStatus = AudioStatus::Stopping;
                }

                soundChannel.Stop(soundChannelSettings.fadeMs.out);
            }
        }
        if (ChannelAvailable(soundChannel))
        {
            auto it = instanceLog_.find(soundWaiting.instance.id);
            assert(it != instanceLog_.end());

            auto& soundWaitingStatus = it->second.second;
            soundWaitingStatus = AudioStatus::Playing;

            soundChannel.SetSoundInstance(soundWaiting.instance);
            soundChannelSettings = std::move(soundWaiting.settings);
            soundWaiting.instance = {};

            soundChannel.Play(soundChannelSettings.loopCount,
                              soundChannelSettings.fadeMs.in);

            if (ValidStageSlot(soundOnDeck))
            {
                std::swap(soundWaiting, soundOnDeck);
            }
        }
    }
}


