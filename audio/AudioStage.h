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
    AudioChannelSettings settings;
    uint8_t force = 0;
};

using SoundStageSlot = AudioStageSlot<Mix_Chunk>;
using MusicStageSlot = AudioStageSlot<Mix_Music>;

template <typename T>
using AudioStageSlotPair = std::pair<AudioStageSlot<T>, AudioStageSlot<T>>;

using SoundStageSlotPair = AudioStageSlotPair<Mix_Chunk>;
using MusicStageSlotPair = AudioStageSlotPair<Mix_Music>;

struct AudioStage
{
    std::array<SoundStageSlotPair, MIX_CHANNELS> stagedSounds;
    MusicStageSlotPair stagedMusic;
    size_t fairSoundForceIdx = 0;
};