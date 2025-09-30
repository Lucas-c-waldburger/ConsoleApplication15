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

    template <typename T>
    size_t StageAudio(AudioStageSlot<T> stageSlot);

    void ClearChannels();
    void ClearStage();
    void UpdateChannels();

private:
    size_t StageMusic(MusicStageSlot stageSlot);
    size_t StageSound(SoundStageSlot stageSlot);

    void UpdateMusicChannel();
    void UpdateSoundChannels();

    AudioChannels channels_;
    AudioStage stage_;
};


template<typename T>
inline size_t AudioManager::StageAudio(AudioStageSlot<T> stageSlot)
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
