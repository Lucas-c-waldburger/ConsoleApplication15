#include "AudioManager.h"


namespace {

template <typename Ch>
static bool ChannelAvailable(const Ch& channel)
{
    return !(channel.IsPlaying() || channel.IsPaused());
}

template <typename Ch>
static bool ShouldStopChannel(const Ch& channel, uint8_t force)
{
    return !ch.IsStopping() &&
        (force & (AudioForcing::ForceChannelGraceful |
            AudioForcing::ForceChannelHalt)) != 0;
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
    musicChannel.SetMusic(nullptr);

    for (auto& [soundChannel, _] : channels_.soundChannels)
    {
        soundChannel.Stop();
        soundChannel.SetSound(nullptr);
    }
}

void AudioManager::ClearStage()
{
    stage_.stagedMusic.first.audioPtr = nullptr;
    stage_.stagedMusic.second.audioPtr = nullptr;

    for (auto& soundStage : stage_.stagedSounds)
    {
        soundStage.first.audioPtr = nullptr;
        soundStage.second.audioPtr = nullptr;
    }
}

void AudioManager::UpdateChannels()
{
    UpdateMusicChannel();
    UpdateSoundChannels();
}

size_t AudioManager::StageMusic(MusicStageSlot stageSlot)
{
    auto& [musicWaiting, musicOnDeck] = stage_.stagedMusic;

    if (!musicWaiting.audioPtr)
    {
        musicWaiting = std::move(stageSlot);
        return kMusicChannelIndex;
    }
    if (!musicOnDeck.audioPtr ||
        (stageSlot.force & AudioForcing::ForceStage) != 0)
    {
        musicOnDeck = std::move(stageSlot);
        return kMusicChannelIndex;
    }

    LOG_WARNING("Music could not be staged, no open slot");
    return kInvalidChannelIndex;
}

size_t AudioManager::StageSound(SoundStageSlot stageSlot)
{
    for (size_t i = 0; i < stage_.stagedSounds.size(); i++)
    {
        auto& [soundWaiting, soundOnDeck] = stage_.stagedSounds[i];

        if (!soundWaiting.audioPtr)
        {
            soundWaiting = std::move(stageSlot);
            return i;
        }
        if (!soundOnDeck.audioPtr)
        {
            soundOnDeck = std::move(stageSlot);
            return i;
        }
    }

    // no available channels, if forcing, overwrite equitably
    if ((stageSlot.force & AudioForcing::ForceStage) != 0)
    {
        size_t channelIdx = stage_.fairSoundForceIdx;

        stage_.stagedSounds[channelIdx].second = std::move(stageSlot);
        stage_.fairSoundForceIdx =
            (channelIdx + 1) % stage_.stagedSounds.size();

        return channelIdx;
    }

    LOG_WARNING("Sound could not be staged, no open slot");
    return kInvalidChannelIndex;
}

void AudioManager::UpdateMusicChannel()
{
    auto& [musicWaiting, musicOnDeck] = stage_.stagedMusic;
    auto& [musicChannel, musicChannelSettings] = channels_.musicChannel;

    if (!musicWaiting.audioPtr)
    {
        if (!musicOnDeck.audioPtr)
        {
            return;
        }
        std::swap(musicWaiting, musicOnDeck);
    }

    if (!ChannelAvailable(musicChannel))
    {
        if (ShouldStopChannel(musicChannel, musicWaiting.force))
        {
            musicChannel.Stop(
                (musicWaiting.force & AudioForcing::ForceChannelHalt) != 0
                ? 0
                : musicChannelSettings.fadeMs.out); // force graceful
        }
    }
    if (ChannelAvailable(musicChannel))
    {
        musicChannel.SetMusic(musicWaiting.audioPtr);
        musicChannelSettings = std::move(musicWaiting.settings);
        musicWaiting.audioPtr = nullptr;

        musicChannel.Play(musicChannelSettings.loopCount,
            musicChannelSettings.fadeMs.in);

        if (musicOnDeck.audioPtr)
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

        if (!soundWaiting.audioPtr)
        {
            if (!soundOnDeck.audioPtr)
            {
                continue;
            }
            std::swap(soundWaiting, soundOnDeck);
        }

        if (!ChannelAvailable(soundChannel))
        {
            if (ShouldStopChannel(soundChannel, soundOnDeck.force))
            {
                soundChannel.Stop(
                    (soundWaiting.force & AudioForcing::ForceChannelHalt) != 0
                    ? 0
                    : soundChannelSettings.fadeMs.out); // force graceful
            }
        }
        if (ChannelAvailable(soundChannel))
        {
            soundChannel.SetSound(soundWaiting.audioPtr);
            soundChannelSettings = std::move(soundWaiting.settings);
            soundWaiting.audioPtr = nullptr;

            soundChannel.Play(soundChannelSettings.loopCount,
                soundChannelSettings.fadeMs.in);

            if (soundOnDeck.audioPtr)
            {
                std::swap(soundWaiting, soundOnDeck);
            }
        }
    }
}


