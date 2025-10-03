#pragma once
#include <array>
#include "AudioSettings.h"
#include "AudioCommon.h"

enum AudioForcing : uint8_t
{
    ForceStage = 1 << 0,
    ForceChannelGraceful = 1 << 1,
    ForceChannelHalt = 1 << 2
};

template <typename T>
struct AudioStageSlot
{
    AudioInstanceResource<T> instance;
    AudioSettings settings;
    uint8_t force = 0;
};

using SoundStageSlot = AudioStageSlot<Mix_Chunk>;
using MusicStageSlot = AudioStageSlot<Mix_Music>;

struct AudioStage
{
    std::array<std::pair<SoundStageSlot, SoundStageSlot>, MIX_CHANNELS> stagedSounds;
    std::pair<MusicStageSlot, MusicStageSlot> stagedMusic;
    size_t fairSoundForceIdx = 0;
};