#include "AudioManager.h"
#include "../core/Result.h"

namespace {

bool IsAudioSpatialDataEmpty(const AudioSpatialData& spatialData)
{
    return !(spatialData.angle.has_value() ||
             spatialData.distance.has_value() ||
             spatialData.panning.has_value());
}

template <SomeMixType T>
constexpr bool EmptyStageSlot(const AudioStageSlot<T>& slot)
{
    if (!slot.instance.audioPtr)
    {
        assert(!slot.instance.id.IsValid());
        return true;
    }

    assert(slot.instance.id.IsValid());
    return false;
}

template <SomeMixType T>
constexpr bool ValidStageSlot(const AudioStageSlot<T>& slot)
{
    return slot.instance.audioPtr != nullptr && slot.instance.id.IsValid();
}

template <SomeAudioChannel T>
bool ChannelAvailable(const T& channel)
{
    return !(channel.IsPlaying() || channel.IsPaused());
}

template <SomeAudioChannel T>
bool ShouldForceChannelStop(const T& channel, uint8_t force)
{
    constexpr uint8_t eitherChannelForcing = (AudioForcing::ForceChannelGraceful |
                                              AudioForcing::ForceChannelHalt);
    // Rethink this for force halt
    return !channel.IsStopping() && ((force & eitherChannelForcing) != 0);
}
template <SomeMixType T>
constexpr bool StagingForced(const AudioStageSlot<T>& slot)
{
    return (slot.force & AudioForcing::ForceStage) != 0;
}

template <SomeMixType T>
void TransferStagedSettings(AudioStageSlot<T>& stageSlot, 
                            AudioUpdateSettings&& updateSettings)
{
    auto& slotSettings = stageSlot.settings;
    auto& [volume, loopCount, fadeMs, spatial, trackPos] = updateSettings;

    slotSettings.volume = volume.value_or(slotSettings.volume);
    slotSettings.loopCount = loopCount.value_or(slotSettings.loopCount);
    slotSettings.fadeMs = fadeMs.value_or(slotSettings.fadeMs);

    if constexpr (std::same_as<T, Mix_Music>)
    {
        return;
    }
    else
    {
        auto& [angle, distance, panning] = updateSettings.spatial;

        slotSettings.spatial.angle = (angle.has_value()) ? 
            angle : slotSettings.spatial.angle;
        slotSettings.spatial.distance = (distance.has_value()) ? 
            distance : slotSettings.spatial.distance;
        slotSettings.spatial.panning = (panning.has_value()) ? 
            panning : slotSettings.spatial.panning;
    }
}

template <SomeAudioChannel T> 
void TransferChannelSettingsAndApply(AudioChannelSettingsPair<T>& channelSettingsPair,
                                     AudioUpdateSettings&& updateSettings)
{
    auto& [channel, channelSettings] = channelSettingsPair;
    auto& [volume, loopCount, fadeMs, spatial, trackPos] = updateSettings;

    if (volume.has_value() && *volume != channelSettings.volume)
    {
        channel.SetVolume(*volume);
        channelSettings.volume = *volume;
    }

    channelSettings.loopCount = loopCount.value_or(channelSettings.loopCount);
    channelSettings.fadeMs = fadeMs.value_or(channelSettings.fadeMs);

    if constexpr (std::same_as<T, MusicChannel>)
    {
        return;
    }
    else
    {
        auto& [angle, distance, panning] = updateSettings.spatial;

        if (angle.has_value() && angle != channelSettings.spatial.angle)
        {
            uint8_t resolvedDistance =
                distance.value_or(channelSettings.spatial.distance.value_or(0));

            channel.SetPosition(*angle, resolvedDistance);
            channelSettings.spatial.angle = angle;
            channelSettings.spatial.distance = resolvedDistance;
            channelSettings.spatial.panning.reset(); // angle overrides panning

            return;
        }
        if (distance.has_value() && distance != channelSettings.spatial.distance)
        {
            channel.SetDistance(*distance);
            channelSettings.spatial.distance = *distance;
        }
        if (panning.has_value() && panning != channelSettings.spatial.panning)
        {
            channel.SetPanning(panning->left, panning->right);
            channelSettings.spatial.panning = *panning;
        }
    }
}

template <SomeAudioChannel T>
AudioStatus GetChannelAudioStatus(T& channel)
{
    if (!channel.HasAudioInstance())
    {
        return AudioStatus::Stopped;
    }
    if (channel.IsPlaying())
    {
        if (channel.IsStopping())
        {
            return AudioStatus::Stopping;
        }

        return AudioStatus::Playing;
    }

    return (channel.IsPaused()) ? AudioStatus::Paused : AudioStatus::Stopped;
}

template <SomeAudioChannel T>
const AudioChannelSettings& 
GetChannelAudioSettings(const AudioInstanceID& instanceId,
                        const AudioChannelSettingsPair<T>& channelSettingsPair)
{
    auto& [channel, channelSettings] = channelSettingsPair;

    assert(channel.GetActiveAudioInstance().id == instanceId);

    return channelSettings;
}

template <SomeMixType T>
const AudioChannelSettings& GetStagedAudioSettings(const AudioInstanceID& instanceId, 
                                                   const AudioStageSlotPair<T>& slots)
{
    auto& [audioWaiting, audioOnDeck] = slots;

    auto& matchingSlot = (audioWaiting.instance.id == instanceId)
        ? audioWaiting : audioOnDeck;

    assert(matchingSlot.instance.id == instanceId);

    return matchingSlot.settings;
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

void AudioManager::UpdateChannels(float dt)
{
    UpdateMusicChannel();
    UpdateSoundChannels(dt);
}

AudioManager::InstanceChannelAndStatus 
AudioManager::GetAudioInstanceChannelAndStatus(const AudioInstanceID& instanceId) const
{
    auto it = instanceLog_.find(instanceId);

    return (it != instanceLog_.end()) ? it->second : kInvalidInstanceChannelAndStatus;
}

const AudioChannelSettings& 
AudioManager::GetInstanceAudioSettings(const AudioInstanceID& instanceId) const
{
    auto it = instanceLog_.find(instanceId);
    if (it == instanceLog_.end())
    {
        LOG_ERROR("Could not retrieve instance's audio settings: "
            "Instance Id not found in log");

        return kInvalidAudioChannelSettings;
    }

    const auto [channelIdx, status] = it->second;
    assert(channelIdx <= kMusicChannelIndex);

    if (status == AudioStatus::Staged)
    {
        return (channelIdx == kMusicChannelIndex)
            ? GetStagedAudioSettings(instanceId, stage_.stagedMusic)
            : GetStagedAudioSettings(instanceId, stage_.stagedSounds[channelIdx]);
    }

    // not staged, should be in a channel
    return (channelIdx == kMusicChannelIndex)
        ? GetChannelAudioSettings(instanceId, channels_.musicChannel)
        : GetChannelAudioSettings(instanceId, channels_.soundChannels[channelIdx]);
}

void AudioManager::UpdateAudioSettings(const AudioInstanceID& instanceId,
                                       AudioUpdateSettings&& newSettings)
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

            TransferStagedSettings(matchingSlot, std::move(newSettings));

            return;
        }
        else
        {
            auto& [soundWaiting, soundOnDeck] = stage_.stagedSounds[channelIdx];

            auto& matchingSlot = (soundWaiting.instance.id == instanceId)
                ? soundWaiting : soundOnDeck;

            assert(matchingSlot.instance.id == instanceId);

            TransferStagedSettings(matchingSlot, std::move(newSettings));

            return;
        }
    }

    // not staged, should be in a channel
    if (channelIdx == kMusicChannelIndex)
    {
        assert(channels_.musicChannel.first.GetActiveAudioInstance().id == instanceId);

        TransferChannelSettingsAndApply(channels_.musicChannel, std::move(newSettings));
    }
    else
    {
        assert(channels_.soundChannels[channelIdx].first.GetActiveAudioInstance().id == instanceId);

        TransferChannelSettingsAndApply(channels_.soundChannels[channelIdx], std::move(newSettings));
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

    auto& [channelIdx, status] = it->second;
    assert(channelIdx <= kMusicChannelIndex);

    if (command == AudioPlayCommand::None)
    {
        return status;
    }

    if (status == AudioStatus::Staged)
    {
        // if staged, only care about Stop command (it means "unstage" in this context)
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
        break;

    case AudioPlayCommand::Resume:
        (channelIdx == kMusicChannelIndex)
            ? channels_.musicChannel.first.Resume()
            : channels_.soundChannels[channelIdx].first.Resume();

        status = AudioStatus::Playing;
        break;

    case AudioPlayCommand::Restart:
        (channelIdx == kMusicChannelIndex)
            ? channels_.musicChannel.first.Play(
                channels_.musicChannel.second.loopCount,
                channels_.musicChannel.second.fadeMs.out)
            : channels_.soundChannels[channelIdx].first.Play(
                channels_.soundChannels[channelIdx].second.loopCount,
                channels_.soundChannels[channelIdx].second.fadeMs.out);

        status = AudioStatus::Playing;
        break;

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
            break;
        }
    }
    default:
        assert(false);
    }

    return status;
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
    if (!instanceId.IsValid())
    {
        return false;
    }

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
    if (EmptyStageSlot(musicOnDeck) || StagingForced(stageSlot))
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
        LOG_ERROR("Sound not staged: Duplicate instance id in log");

        return kInvalidInstanceChannelAndStatus;
    }

    size_t onDeckBackupIdx = kInvalidChannelIndex;
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

        // stash a found on-deck slot in case all 'waiting' stage positions are full
        if (EmptyStageSlot(soundOnDeck) && onDeckBackupIdx == kInvalidChannelIndex)
        {
            onDeckBackupIdx = i;
        }
    }

    // no 'waiting' channels available, see if we found an on-deck slot to use
    if (onDeckBackupIdx != kInvalidChannelIndex)
    {
        assert(onDeckBackupIdx < kMusicChannelIndex);
        auto& foundOnDeckSlot = stage_.stagedSounds[onDeckBackupIdx].second;

        auto logEntry = std::make_pair(onDeckBackupIdx, AudioStatus::Staged);
        instanceLog_[stageSlot.instance.id] = logEntry;
        foundOnDeckSlot = std::move(stageSlot);

        return logEntry;
    }

    // no available waiting or on-deck slots. if forcing, overwrite equitably
    if (StagingForced(stageSlot))
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
    auto& [musicChannel, musicChannelSettings] = channels_.musicChannel;

    auto updateTrackPos = [&musicChannel, &musicChannelSettings]() {
        if (musicChannel.IsPlaying())
        {
            musicChannelSettings.trackPosition =
                static_cast<float>(musicChannel.GetTrackPositionSec());
        }
    };

    // cleanup instance if music done
    if (ChannelAvailable(musicChannel) && musicChannel.HasAudioInstance())
    {
        size_t erased = instanceLog_.erase(musicChannel.GetActiveAudioInstance().id);
        assert(erased > 0);
        musicChannel.SetMusicInstance({});
    }

    // if nothing on stage, jump to update track pos. 
    // if on-after stage full but on-next stage clear, move on-after up
    {
        // scope this block since musicOnNext and musicOnAfter may swap and become stale
        auto& [musicOnNext, musicOnAfter] = stage_.stagedMusic;

        if (EmptyStageSlot(musicOnNext))
        {
            if (EmptyStageSlot(musicOnAfter))
            {
                updateTrackPos();

                return;
            }

            std::swap(musicOnNext, musicOnAfter);
        }
    }

    auto& [musicOnNext, musicOnAfter] = stage_.stagedMusic;

    // if music playing, check to see if we should stop it
    if (!ChannelAvailable(musicChannel))
    {
        if (ShouldForceChannelStop(musicChannel, musicOnNext.force))
        {
            auto it = instanceLog_.find(musicChannel.GetActiveAudioInstance().id);
            assert(it != instanceLog_.end());

            auto& [instanceChannel, instanceStatus] = it->second;
            assert(instanceChannel == kMusicChannelIndex);
            assert(instanceStatus != AudioStatus::Staged);

            if ((musicOnNext.force & AudioForcing::ForceChannelHalt) != 0 ||
                musicChannel.IsPaused()) // fade out not relevant if paused
            {
                musicChannelSettings.fadeMs.out = 0;
            }

            const bool willStopImmediately = (musicChannelSettings.fadeMs.out <= 0);

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

    // if music had no fade or forced halt, put musicOnNext on channel and play
    if (ChannelAvailable(musicChannel))
    {
        auto it = instanceLog_.find(musicOnNext.instance.id);
        assert(it != instanceLog_.end());

        auto& musicOnNextStatus = it->second.second;
        assert(musicOnNextStatus == AudioStatus::Staged);
        musicOnNextStatus = AudioStatus::Playing;

        musicChannel.SetMusicInstance(musicOnNext.instance);

        musicChannelSettings = std::move(musicOnNext.settings);
        musicChannelSettings.trackPosition = 0.0f;

        musicOnNext.instance = {};
       
        musicChannel.Play(musicChannelSettings.loopCount,
                          musicChannelSettings.fadeMs.in);

        if (ValidStageSlot(musicOnAfter))
        {
            std::swap(musicOnNext, musicOnAfter);
        }
    }

    updateTrackPos();
}

void AudioManager::UpdateSoundChannels(float dt)
{
    for (size_t i = 0; i < MIX_CHANNELS; i++)
    {
        auto& [soundChannel, soundChannelSettings] = channels_.soundChannels[i];

        auto updateTrackPos = [&soundChannel, &soundChannelSettings, dt]() {
            if (soundChannel.IsPlaying())
            {
                // use dt since chunk api doesn't report track pos
                soundChannelSettings.trackPosition += dt;
            }
        };

        if (ChannelAvailable(soundChannel) && soundChannel.HasAudioInstance())
        {
            [[maybe_unused]] const size_t erased = 
                instanceLog_.erase(soundChannel.GetActiveAudioInstance().id);
            assert(erased > 0);

            soundChannel.SetSoundInstance({});
        }

        {
            // scope this block since soundOnNext and soundOnAfter may swap and become stale
            auto& [soundOnNext, soundOnAfter] = stage_.stagedSounds[i];

            if (EmptyStageSlot(soundOnNext))
            {
                if (EmptyStageSlot(soundOnAfter))
                {
                    updateTrackPos();

                    continue;
                }
                std::swap(soundOnNext, soundOnAfter);
            }
        }

        auto& [soundOnNext, soundOnAfter] = stage_.stagedSounds[i];

        if (!ChannelAvailable(soundChannel))
        {
            if (ShouldForceChannelStop(soundChannel, soundOnNext.force))
            {
                auto it = instanceLog_.find(soundChannel.GetActiveSoundInstance().id);
                assert(it != instanceLog_.end());

                auto& [instanceChannel, instanceStatus] = it->second;
                assert(instanceChannel == soundChannel.GetChannelIndex());
                assert(instanceStatus != AudioStatus::Staged);

                if ((soundOnNext.force & AudioForcing::ForceChannelHalt) != 0 ||
                    soundChannel.IsPaused()) // fade out not relevant if paused
                {
                    soundChannelSettings.fadeMs.out = 0;
                }

                const bool willStopImmediately = (soundChannelSettings.fadeMs.out <= 0);

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
            auto it = instanceLog_.find(soundOnNext.instance.id);
            assert(it != instanceLog_.end());

            auto& soundOnNextStatus = it->second.second;
            assert(soundOnNextStatus == AudioStatus::Staged);
            soundOnNextStatus = AudioStatus::Playing;

            soundChannel.SetSoundInstance(soundOnNext.instance);

            soundChannelSettings = std::move(soundOnNext.settings);
            soundChannelSettings.trackPosition = 0.0f;

            soundOnNext.instance = {};

            soundChannel.Play(soundChannelSettings.loopCount,
                              soundChannelSettings.fadeMs.in);

            if (ValidStageSlot(soundOnAfter))
            {
                std::swap(soundOnNext, soundOnAfter);
            }
        }

        updateTrackPos();
    }
}

const MusicChannel& AudioManager::GetMusicChannel() const
{
    return channels_.musicChannel.first;
}

const SoundChannel& AudioManager::GetSoundChannel(size_t channelIdx) const
{
    assert(channelIdx < MIX_CHANNELS);

    return channels_.soundChannels[channelIdx].first;
}
