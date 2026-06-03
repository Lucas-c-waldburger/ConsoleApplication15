#include "AudioBank2.h"

Result<Handle<Audio2>> AudioBank2::LoadAudio(AudioDescriptor&& desc)
{
    if (nameToInfoIdx_.contains(desc.name))
    {
		return MAKE_ERROR_FMT("Audio with name '{}' already exists in the bank", desc.name);
    }

    auto impl = [this, &desc](auto& ptrContainer, auto& ptrMakerFunc) 
    -> Result<Handle<Audio2>> {
        size_t newStorageIdx = ptrContainer.size();

        auto newPtr = ptrMakerFunc(desc.filepath);
        if (!newPtr)
        {
            return MAKE_ERROR(Mix_GetError());
        }

        ptrContainer.emplace_back(std::move(newPtr));

        auto [_, inserted] = nameToInfoIdx_.try_emplace(desc.name, newStorageIdx);
        assert(inserted);

        const size_t newResourceIdx = audioInfo_.PushBack({
            .audioType = desc.audioType,
            .name = std::move(desc.name),
			.filepath = std::move(desc.filepath),
            .storageIndex = newStorageIdx
		});

        Handle<Audio2> newHandle = Handle<Audio2>::Create(instanceId_, newResourceIdx);

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

bool AudioBank2::HasAudio(std::string_view name) const
{
    return nameToInfoIdx_.contains(name);
}

Result<SoundInstanceResource> AudioBank2::GetSoundInstanceResouce(const Handle<Audio2>& handle)
{
    return GetAudioInstanceDataInternal<Mix_Chunk>(handle);
}

Result<MusicInstanceResource> AudioBank2::GetMusicInstanceResource(const Handle<Audio2>& handle)
{
    return GetAudioInstanceDataInternal<Mix_Music>(handle);
}

std::vector<AudioDescriptor> AudioBank2::ExportAudioDescriptors() const
{
    std::vector<AudioDescriptor> descriptors;
    descriptors.reserve(audioInfo_.Size());
    
    for (const auto& [type, nm, fp] : audioInfo_.ForEach<&AudioInfo::audioType, 
                                                         &AudioInfo::name, 
                                                         &AudioInfo::filepath>())
    {
        descriptors.emplace_back(AudioDescriptor{
            .audioType = type,
            .name = nm,
            .filepath = fp
        });
	}

	return descriptors; 
 }