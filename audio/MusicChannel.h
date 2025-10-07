#pragma once
#include <SDL_mixer.h>
#include <algorithm>
#include "AudioInstance.h"

class MusicChannel
{
public:
    static constexpr size_t kChannelIndex = MIX_CHANNELS;

    MusicChannel() : activeMusic_(nullptr) {}

    void SetMusicInstance(const MusicInstanceResource& music) { activeMusic_ = music; }
    const MusicInstanceResource& GetActiveAudioInstance() const { return activeMusic_; }
    bool HasAudioInstance() const
    {
        return activeMusic_.audioPtr != nullptr && activeMusic_.id.IsValid();
    }

    int GetVolume() const { return Mix_GetMusicVolume(activeMusic_.audioPtr); }
    void SetVolume(int volume)
    {
        Mix_VolumeMusic(std::clamp(volume, 0, MIX_MAX_VOLUME));
    }

    bool IsPlaying() const 
    { 
        return Mix_PlayingMusic() != 0 &&
               Mix_PausedMusic() == 0; 
    }
    bool IsPaused() const { return Mix_PausedMusic() != 0; }
    bool IsStopping() const
    {
        return IsFadingOut();
    }
    bool IsFadingIn() const
    {
        return (Mix_FadingMusic() == MIX_FADING_IN);
    }
    bool IsFadingOut() const
    {
        return (Mix_FadingMusic() == MIX_FADING_OUT);
    }

    void Pause() { Mix_PauseMusic(); }
    void Resume() { Mix_ResumeMusic(); }
    void Play(int loops, int fadeInMs = 0)
    {
        if (fadeInMs > 0)
        {
            Mix_FadeInMusic(activeMusic_.audioPtr, loops, fadeInMs);
        }
        else
        {
            Mix_PlayMusic(activeMusic_.audioPtr, loops);
        }
    }
    void Stop(int fadeOutMs = 0)
    {
        if (fadeOutMs > 0)
        {
            Mix_FadeOutMusic(fadeOutMs);
        }
        else
        {
            Mix_HaltMusic();
        }
    }
    void FadeIn(int fadeInMs, int loops)
    {
        Mix_FadeInMusic(activeMusic_.audioPtr, loops, fadeInMs);
    }
    void FadeOut(int fadeOutMs)
    {
        Mix_FadeOutMusic(fadeOutMs);
    }

    double GetDurationSec() const
    {
        return Mix_MusicDuration(activeMusic_.audioPtr);
    }
    int GetDurationMs() const
    {
        return static_cast<int>(Mix_MusicDuration(activeMusic_.audioPtr) * 1000.0);
    }

    double GetTrackPositionSec() const
    {
        return Mix_GetMusicPosition(activeMusic_.audioPtr);
    }
    int GetTrackPositionMs() const
    {
        return static_cast<int>(Mix_GetMusicPosition(activeMusic_.audioPtr) * 1000.0);
    }

    void SetTrackPositionSec(double sec)
    {
        Mix_SetMusicPosition(sec);
    }
    void SetTrackPositionMs(int ms)
    {
        Mix_SetMusicPosition(static_cast<double>(ms) / 1000.0);
    }

    size_t GetChannelIndex() const { return kChannelIndex; }

private:
    MusicInstanceResource activeMusic_;
};
