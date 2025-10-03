#pragma once
#include <SDL_mixer.h>
#include <algorithm>
#include "AudioInstance.h"

class SoundChannel
{
public:
    SoundChannel() : activeSound_(), channelIndex_(0) {}
    explicit SoundChannel(size_t channelIndex) : 
        activeSound_(), channelIndex_(channelIndex) {}

    void SetSoundInstance(const SoundInstanceResource& sound) { activeSound_ = sound; }
    const SoundInstanceResource& GetSoundInstance() const { return activeSound_; }
    bool HasSoundInstance() const 
    { 
        return activeSound_.audioPtr != nullptr && activeSound_.id.IsValid();
    }

    int GetVolume() const { return Mix_VolumeChunk(activeSound_.audioPtr, -1); }
    void SetVolume(int volume)
    {
        Mix_VolumeChunk(activeSound_.audioPtr, std::clamp(volume, 0, MIX_MAX_VOLUME));
    }

    bool IsPlaying() const { return Mix_Playing(channelIndex_) != 0; }
    bool IsPaused() const { return Mix_Paused(channelIndex_) != 0; }
    bool IsStopping() const
    {
        if (Mix_Playing(channelIndex_) == 0)
        {
            isStopping_ = false;
        }
        return isStopping_;
    }

    void Pause() { Mix_Pause(channelIndex_); }
    void Resume() { Mix_Resume(channelIndex_); }
    void Play(int loops, int fadeInMs = 0)
    {
        if (fadeInMs > 0)
        {
            Mix_FadeInChannel(channelIndex_, activeSound_.audioPtr, loops, fadeInMs);
        }
        else
        {
            Mix_PlayChannel(channelIndex_, activeSound_.audioPtr, loops);
        }
    }
    void Stop(int fadeOutMs = 0)
    {
        if (fadeOutMs > 0)
        {
            Mix_FadeOutChannel(channelIndex_, fadeOutMs);
            isStopping_ = true;
        }
        else
        {
            Mix_HaltChannel(channelIndex_);
        }
    }

    void FadeIn(int fadeInMs, int loops)
    {
        Mix_FadeInChannel(channelIndex_, activeSound_.audioPtr, loops, fadeInMs);
    }
    void FadeOut(int fadeOutMs)
    {
        Mix_FadeOutChannel(channelIndex_, fadeOutMs);
    }

    void SetPanning(uint8_t left, uint8_t right) 
    { 
        Mix_SetPanning(channelIndex_, left, right);
    }
    void SetDistance(uint8_t distance) { Mix_SetDistance(channelIndex_, distance); }
    void SetPosition(int16_t angle, uint8_t distance)
    {
        Mix_SetPosition(channelIndex_, angle, distance);
    }

    size_t GetChannelIndex() const { return channelIndex_; }

    const SoundInstanceResource& GetActiveSoundInstance() const { return activeSound_; }

private:
    size_t channelIndex_;
    SoundInstanceResource activeSound_;
    mutable bool isStopping_ = false;
};