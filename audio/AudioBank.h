#pragma once
#include <unordered_map>
#include <vector>
#include <cassert>
#include "../core/Result.h"
#include "AudioHandle.h"


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

    Result<Mix_Chunk*> GetSound(const Handle<Audio>& handle);
    Result<Mix_Music*> GetMusic(const Handle<Audio>& handle);

    AudioDescriptor* GetAudioDescriptor(const Handle<Audio>& handle);

private:
    template <typename T>
        requires (std::same_as<T, Mix_Chunk> || std::same_as<T, Mix_Music>)
    Result<T*> GetAudioInternal(const Handle<Audio>& handle);

    using DescriptorMap = std::unordered_map<Handle<Audio>,
        std::pair<AudioDescriptor, size_t>>;

    DescriptorMap indexedDescriptors_;
    std::vector<SoundPtr> sounds_;
    std::vector<MusicPtr> music_;
};


template <typename T>
    requires (std::same_as<T, Mix_Chunk> || std::same_as<T, Mix_Music>)
inline Result<T*> AudioBank::GetAudioInternal(const Handle<Audio>& handle)
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

    auto findPtr = [&]
    (auto& ptrContainer, AudioType expectedType) -> Result<T*>
        {
            if (handle.GetAudioType() != expectedType)
            {
                return MAKE_ERROR("Audio handle did not have expected audio type");
            }
            if (idx >= ptrContainer.size())
            {
                return MAKE_ERROR("Mapped index for audio ptr out of range");
            }

            return ptrContainer[idx].get();
        };

    if constexpr (std::same_as<T, Mix_Chunk>)
    {
        return findPtr(sounds_, AudioType::Sound);
    }
    else // music
    {
        return findPtr(music_, AudioType::Music);
    }
}
