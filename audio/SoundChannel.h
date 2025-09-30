#pragma once
#include <SDL_mixer.h>
#include <algorithm>

class SoundChannel
{
public:
    SoundChannel() : activeSound_(nullptr), channelIndex_(0) {}
    explicit SoundChannel(size_t channelIndex) : 
        activeSound_(nullptr), channelIndex_(channelIndex) {}

    void SetSound(Mix_Chunk* soundChunk) { activeSound_ = soundChunk; }
    bool HasSound() const { return activeSound_ != nullptr; }

    int GetVolume() const { return Mix_VolumeChunk(activeSound_, -1); }
    void SetVolume(int volume)
    {
        Mix_VolumeChunk(activeSound_, std::clamp(volume, 0, MIX_MAX_VOLUME));
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
            Mix_FadeInChannel(channelIndex_, activeSound_, loops, fadeInMs);
        }
        else
        {
            Mix_PlayChannel(channelIndex_, activeSound_, loops);
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
        Mix_FadeInChannel(channelIndex_, activeSound_, loops, fadeInMs);
    }
    void FadeOut(int fadeOutMs)
    {
        Mix_FadeOutChannel(channelIndex_, fadeOutMs);
    }

    void SetDistance(uint8_t distance) { Mix_SetDistance(channelIndex_, distance); }

    size_t GetChannelIndex() const { return channelIndex_; }

private:
    size_t channelIndex_;
    Mix_Chunk* activeSound_;
    mutable bool isStopping_ = false;
};