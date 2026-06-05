#pragma once
#include <SDL_mixer.h>
#include <memory>
#include <string>

using SoundPtr = std::unique_ptr < Mix_Chunk,
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
