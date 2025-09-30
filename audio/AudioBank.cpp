#include "AudioBank.h"

Result<Handle<Audio>> AudioBank::LoadAudio(AudioDescriptor&& desc)
{
    auto impl = [this, &desc]
    (auto& ptrContainer, auto& ptrMakerFunc) -> Result<Handle<Audio>>
    {
        size_t newIdx = ptrContainer.size();

        auto newPtr = ptrMakerFunc(desc.filepath);
        if (!newPtr)
        {
            return MAKE_ERROR("Failed to create audio pointer");
        }

        ptrContainer.emplace_back(std::move(newPtr));

        Handle<Audio> newHandle = Handle<Audio>::Create(desc.audioType);

        indexedDescriptors_.emplace(newHandle,
            std::make_pair(std::move(desc), newIdx)
        );

        return newHandle;
    };

    switch (desc.audioType)
    {
    case AudioType::Sound:
        return impl(sounds_, MakeSoundPtr);
    case AudioType::Music:
        return impl(music_, MakeMusicPtr);
    case AudioType::Unknown: default:
        return MAKE_ERROR("Audio descriptor's audio type was Unknown");
    }
}

bool AudioBank::HasAudio(const Handle<Audio>& handle) const
{
    auto it = indexedDescriptors_.find(handle);
    if (it != indexedDescriptors_.end())
    {
        const size_t idx = it->second.second;

        switch (it->second.first.audioType)
        {
        case AudioType::Sound:
            assert(idx < sounds_.size());
            return sounds_[idx] != nullptr;
        case AudioType::Music:
            assert(idx < music_.size());
            return music_[idx] != nullptr;
        case AudioType::Unknown: default:
            return false;
        }
    }

    return false;
}

Result<Mix_Chunk*> AudioBank::GetSound(const Handle<Audio>& handle)
{
    return GetAudioInternal<Mix_Chunk>(handle);
}

Result<Mix_Music*> AudioBank::GetMusic(const Handle<Audio>& handle)
{
    return GetAudioInternal<Mix_Music>(handle);
}

AudioDescriptor* AudioBank::GetAudioDescriptor(const Handle<Audio>& handle)
{
    auto it = indexedDescriptors_.find(handle);
    return (it != indexedDescriptors_.end()) ? &it->second.first : nullptr;
}