#pragma once
#include "../core/Handle.h"
#include "../core/commonObjects.h"
#include <SDL_mixer.h>
#include <memory>
#include <string>

enum class AudioType
{
    Unknown,
    Sound,
    Music
};

enum class AudioPlayCommand
{
    None,
    Pause,
    Resume,
    Restart,
    Stop
};

enum class AudioStatus
{
    Playing,
    Paused,
    Stopping,
    Stopped,
    Staged
};

struct AudioSpatialData
{
    std::optional<int16_t> angle;
    std::optional<uint8_t> distance;
    std::optional<HandedPair<uint8_t>> panning;

    friend constexpr bool operator==(const AudioSpatialData& lhs, 
                                     const AudioSpatialData& rhs)
    {
        return lhs.angle == rhs.angle && lhs.distance == rhs.distance &&
               lhs.panning == rhs.panning;
    }
};

using SoundPtr = std::unique_ptr<Mix_Chunk,
    decltype([](Mix_Chunk* chunk) { if (chunk) { Mix_FreeChunk(chunk); } }) > ;

inline SoundPtr MakeSoundPtr(const std::string& filepath)
{
    return SoundPtr{ Mix_LoadWAV(filepath.c_str()) };
}

using MusicPtr = std::unique_ptr < Mix_Music,
    decltype([](Mix_Music* music) { if (music) { Mix_FreeMusic(music); } }) > ;

inline MusicPtr MakeMusicPtr(const std::string& filepath)
{
    return MusicPtr{ Mix_LoadMUS(filepath.c_str()) };
}

template <typename T>
concept SomeAudioPtr = std::same_as<T, SoundPtr> || std::same_as<T, MusicPtr>;
