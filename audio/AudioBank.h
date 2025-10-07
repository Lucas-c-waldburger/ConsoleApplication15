#pragma once
#include <unordered_map>
#include <vector>
#include <cassert>
#include "../core/Result.h"
#include "AudioHandle.h"
#include "AudioInstance.h"

struct AudioDescriptor
{
    AudioType audioType = AudioType::Sound;
    std::string name;
    std::string filepath;
};

class AudioBank
{
public:
    Result<Handle<Audio>> LoadAudio(AudioDescriptor&& desc);

    bool HasAudio(const Handle<Audio>& handle) const;

    Result<SoundInstanceResource> GetSoundInstanceResouce(const Handle<Audio>& handle);
    Result<MusicInstanceResource> GetMusicInstanceResource(const Handle<Audio>& handle);

    AudioDescriptor* GetAudioDescriptor(const Handle<Audio>& handle);

private:
    template <typename T> requires (std::same_as<T, Mix_Chunk> || 
                                    std::same_as<T, Mix_Music>)
    Result<AudioInstanceResource<T>> GetAudioInstanceDataInternal(const Handle<Audio>& handle);

    using DescriptorMap = std::unordered_map<Handle<Audio>,
        std::pair<AudioDescriptor, size_t>>;

    DescriptorMap indexedDescriptors_;
    std::vector<SoundPtr> sounds_;
    std::vector<MusicPtr> music_;
};


template <typename T> requires (std::same_as<T, Mix_Chunk> || 
                                std::same_as<T, Mix_Music>)
inline Result<AudioInstanceResource<T>> 
AudioBank::GetAudioInstanceDataInternal(const Handle<Audio>& handle)
{
    if (!handle.IsValid())
    {
        return MAKE_ERROR("Audio handle was invalid");
    }

    auto it = indexedDescriptors_.find(handle);
    if (it == indexedDescriptors_.end())
    {
        return MAKE_ERROR("Audio handle not found in sound bank");
    }

    const size_t idx = it->second.second;

    auto makeInstanceData = [&]
    (auto& ptrContainer, AudioType expectedType) -> Result<AudioInstanceResource<T>>
    {
        if (handle.GetAudioType() != expectedType)
        {
            return MAKE_ERROR("Audio handle did not have expected audio type");
        }
        if (idx >= ptrContainer.size())
        {
            return MAKE_ERROR("Mapped index for audio ptr out of range");
        }

        return AudioInstanceResource<T>{
            .audioPtr = ptrContainer[idx].get(),
            .id = AudioInstanceID::Create()
        };
    };

    if constexpr (std::same_as<T, Mix_Chunk>)
    {
        return makeInstanceData(sounds_, AudioType::Sound);
    }
    else // music
    {
        return makeInstanceData(music_, AudioType::Music);
    }
}
