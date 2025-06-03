//#pragma once
//#include <SDL_mixer.h>
//#include <iostream>
//#include <optional>
//#include <algorithm>
//#include "../core/Handle.h"
//#include "../core/Result.h"
//#include "../core/commonObjects.h"
//#include "../deps/function2/function2.hpp"
//#include "../components/BaseComponent.h"
//
//
//using ChunkPtr = std::unique_ptr<Mix_Chunk,
//    decltype([](Mix_Chunk* chunk) { if (chunk) { Mix_FreeChunk(chunk); } })>;
//
//inline ChunkPtr MakeChunkPtr(const std::string& filepath)
//{
//    return ChunkPtr{ Mix_LoadWAV(filepath.c_str()) };
//}
//
//using MusicPtr = std::unique_ptr<Mix_Music,
//    decltype([](Mix_Music* music) { if (music) { Mix_FreeMusic(music); } })>;
//
//inline MusicPtr MakeMusicPtr(const std::string& filepath)
//{
//    return MusicPtr{ Mix_LoadMUS(filepath.c_str()) };
//}
//
//struct SoundSettings
//{
//    int volume = MIX_MAX_VOLUME / 2;
//    int loopCount = 0;
//    struct { int in = 0, out = 0; } fadeMs;
//    int channel = -1;
//    bool spatialized = false;
//};
//
//struct SoundSettings
//{
//    int volume = MIX_MAX_VOLUME / 2;
//    int loopCount = 0;
//    struct 
//    { 
//        int in = 0;
//        int out = 0; 
//    } fadeMs;
//    int channel = -1;
//    struct {
//        bool enabled = false;
//        float maxDistance = 0.0f;
//    } spatialized;
//};
//
//struct Sound
//{
//    std::string name;
//    SoundSettings settings;
//    ChunkPtr chunk;
//    Handle<Sound> handle;
//
//    static Result<Sound> Load(std::string filepath, SoundSettings settings)
//    {
//        auto chunk = MakeChunkPtr(filepath);
//        if (!chunk)
//        {
//            return MAKE_ERROR(Mix_GetError());
//        }
//
//        return Sound{
//            .settings = std::move(settings),
//            .chunk = std::move(chunk),
//            .handle = Handle<Sound>::Create()
//        };
//    }
//};
//
//
//class MixChannel
//{
//public:
//    explicit MixChannel(size_t idx) : channelIndex_(idx) {}
//
//    int GetVolume() const
//    {
//        return Mix_VolumeChunk(activeSound_->chunk.get(), -1);
//    }
//    void SetVolume(int volume)
//    {
//        Mix_VolumeChunk(activeSound_->chunk.get(), std::clamp(volume, 0, MIX_MAX_VOLUME));
//    }
//    bool IsPlaying() const
//    {
//        return Mix_Playing(channelIndex_) != 0;
//    }
//    void Pause()
//    {
//        Mix_Pause(channelIndex_);
//    }
//    void Resume()
//    {
//        Mix_Resume(channelIndex_);
//    }
//    void StartFadeOut(std::optional<int> overrideMs = {})
//    {
//        if (!activeSound_)
//        {
//            return;
//        }
//
//        Mix_FadeOutChannel(channelIndex_, overrideMs.value_or(activeSound_->settings.fadeMs.out));
//    }
//
//    Handle<Sound> GetActiveSound() const
//    {
//        return activeSound_ ? activeSound_->handle : Handle<Sound>{};
//    }
//    void ClearActiveSound()
//    {
//        activeSound_ = nullptr;
//    }
//
//private:
//    size_t channelIndex_ = 0;
//    Sound* activeSound_ = nullptr;
//};
//
//class OnMixChannelFinished
//{
//public:
//    template <typename Fn>
//    static void SetDelegate(Fn&& fn)
//    {
//        instanceCallback_ = std::forward<Fn>(fn);
//        Mix_ChannelFinished(&OnMixChannelFinished::Forward);
//    }
//
//private:
//    static void Forward(int channel)
//    {
//        if (instanceCallback_)
//        {
//            instanceCallback_(channel);
//        }
//    }
//
//    static inline fu2::unique_function<void(int)> instanceCallback_;
//};
//
//class SoundBank
//{
//public:
//    Result<Handle<Sound>> LoadSound(std::string filepath, SoundSettings settings)
//    {
//        auto chunk = MakeChunkPtr(filepath);
//        if (!chunk)
//        {
//            return MAKE_ERROR(Mix_GetError());
//        }
//
//        auto handle = Handle<Sound>::Create();
//
//        bank_[handle] = Sound{
//            .settings = std::move(settings),
//            .chunk = std::move(chunk),
//        };
//    }
//
//    bool UnloadSound(const Handle<Sound>& handle)
//    {
//        return static_cast<bool>(bank_.erase(handle));
//    }
//
//    Sound* GetSound(const Handle<Sound>& handle)
//    {
//        auto it = bank_.find(handle);
//
//        return (it != bank_.end()) ? &it->second : nullptr;
//    }
//
//    SoundSettings* EditSoundSettings(const Handle<Sound>& handle)
//    {
//        auto it = bank_.find(handle);
//
//        return (it != bank_.end()) ? &it->second.settings : nullptr;
//    }
//
//private:
//    std::unordered_map<Handle<Sound>, Sound> bank_;
//};
//
//struct SoundRequest
//{
//    Handle<Sound> handle;
//    SoundSettings settings;
//    std::optional<int> playingOnChannel;
//};
//
//struct Sounds : public BaseComponent<Sounds, 15>
//{
//    std::vector<SoundRequest> queuedSounds;
//    std::vector<SoundRequest> activeSounds;
//};
//
//
//class AudioManager
//{
//public:
//    AudioManager()
//    {
//        OnMixChannelFinished::SetDelegate([this](int channel) {
//            this->channels_[channel].ClearActiveSound();
//        });
//    }
//    explicit AudioManager(SoundBank bank) : bank_(std::move(bank))
//    {
//        OnMixChannelFinished::SetDelegate([this](int channel) {
//            this->channels_[channel].ClearActiveSound();
//            });
//    }
//
//    //void SetSoundBank(SoundBank& bank
//
//    Result<Void> PlaySound(const Handle<Sound>& handle, int channel = -1)
//    {
//        if (!handle.IsValid())
//        {
//            return MAKE_ERROR("Sound handle was invalid");
//        }
//
//        auto it = soundBank_.find(handle);
//        if (it == soundBank_.end())
//        {
//            return MAKE_ERROR("No sound with provided handle found in bank");
//        }
//
//        auto& [settings, chunk] = it->second;
//        if (!chunk)
//        {
//            return MAKE_ERROR("Sound chunk was null");
//        }
//        
//        Mix_VolumeChunk(chunk.get(), std::clamp(settings.volume, 0, MIX_MAX_VOLUME));
//
//        if (settings.fadeMs.in > 0)
//        {
//            channel = Mix_FadeInChannel(channel, chunk.get(), settings.loopCount, settings.fadeMs.in);
//        }
//        else
//        {
//            channel = Mix_PlayChannel(channel, chunk.get(), settings.loopCount);
//        }
//
//        if (channel == -1)
//        {
//            chunk.reset();
//            return MAKE_ERROR(Mix_GetError());
//        }
//
//        Mix_Volume(channel, settings.volume);
//
//        return Void{};
//    }
//
//private:
//    struct SoundData
//    {
//        Sound sound;
//        std::optional<int> activeChannel;
//    }
//
//    std::array<SoundChannel, MIX_CHANNELS> channels_;
//    SoundBank bank_;
//    //std::unordered_map<Handle<Sound>, Sound> soundBank_;
//};
//
//template <size_t NumChannels>
//class Mixer
//{
//public:
//
//private:
//};