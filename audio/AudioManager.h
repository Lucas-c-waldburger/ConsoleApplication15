#pragma once
#include "SoundChannel.h"
#include "MusicChannel.h"
#include "AudioBank.h"
#include "AudioStage.h"

struct AudioChannels
{
    AudioChannels();

    std::array<std::pair<SoundChannel, AudioSettings>, MIX_CHANNELS> soundChannels;
    std::pair<MusicChannel, AudioSettings> musicChannel;
};

class AudioManager
{
public:
    static constexpr size_t kMusicChannelIndex = MIX_CHANNELS + 1;
    static constexpr size_t kInvalidChannelIndex = std::numeric_limits<size_t>::max();

    using InstanceChannelAndStatus = std::pair<size_t, AudioStatus>;

    template <typename T>
    InstanceChannelAndStatus StageAudio(AudioStageSlot<T> stageSlot);

    void ClearChannels();
    void ClearStage();
    void UpdateChannels();

    InstanceChannelAndStatus
    GetAudioInstanceChannelAndStatus(const AudioInstanceID& instanceId) const;

    AudioSettings GetInstanceAudioSettings(const AudioInstanceID& instanceId) const;

    void UpdateAudioSettings(const AudioInstanceID& instanceId, 
                             AudioSettings&& newSettings);

    // returns the instance's status following the execution
    AudioStatus ExecuteAudioCommand(const AudioInstanceID& instanceId,
                                    AudioPlayCommand command);

    void SetSoundInstanceSpatialData(const AudioInstanceID& instanceId,
                                     const AudioSpatialData& spatialData);

    bool AudioInstanceValid(const AudioInstanceID& instanceId) const;

    //void StopOrUnstageAudioInstance(const AudioInstanceID& instanceId);

private:
    using AudioInstanceLog = std::unordered_map<AudioInstanceID, 
                                                InstanceChannelAndStatus>;

    static constexpr InstanceChannelAndStatus kInvalidInstanceChannelAndStatus =
        std::make_pair(kInvalidChannelIndex, AudioStatus::Stopped);

    InstanceChannelAndStatus StageMusic(MusicStageSlot stageSlot);
    InstanceChannelAndStatus StageSound(SoundStageSlot stageSlot);

    void UpdateMusicChannel();
    void UpdateSoundChannels();

    AudioInstanceLog instanceLog_;
    AudioChannels channels_;
    AudioStage stage_;
};


template <typename T>
inline AudioManager::InstanceChannelAndStatus 
AudioManager::StageAudio(AudioStageSlot<T> stageSlot)
{
    if constexpr (std::same_as<T, Mix_Music>)
    {
        return StageMusic(std::move(stageSlot));
    }
    else // Mix_Chunk
    {
        return StageSound(std::move(stageSlot));
    }
}
