#include "AudioBank.h"
#include <filesystem>

//AudioBank::AudioBank(AudioBank&& other) noexcept : audioBankInstanceId_(other.audioBankInstanceId_),
//    sounds_(std::move(other.sounds_)), music_(std::move(other.music_)), 
//    audioInfo_(std::move(other.audioInfo_))
//{
//    RepopulateAudioNameIndexMap(audioInfo_.Capacity());
//}
//
//AudioBank& AudioBank::operator=(AudioBank&& other) noexcept
//{
//    if (this == &other) { return *this; }
//
//    audioBankInstanceId_ = other.audioBankInstanceId_;
//    sounds_ = std::move(other.sounds_);
//    music_ = std::move(other.music_);
//    audioInfo_ = std::move(other.audioInfo_);
//
//    RepopulateAudioNameIndexMap(audioInfo_.Capacity());
//
//    return *this;
//}

Result<Handle<Audio>> AudioBank::LoadAudio(AudioDescriptor&& desc)
{
    if (!std::filesystem::exists(desc.filepath))
    {
        return MAKE_ERROR_FMT("Invalid filepath: '{}'", desc.filepath);
    }

    if (desc.name.empty())
    {
        desc.name = std::filesystem::path(desc.filepath).stem().string();
    }

    if (nameToInfoIdx_.contains(desc.name))
    {
		return MAKE_ERROR_FMT("Audio with name '{}' already exists in the bank", desc.name);
    }

    auto impl = [this, &desc](auto& ptrContainer, auto& ptrMakerFunc) 
    -> Result<Handle<Audio>> {
        size_t newStorageIdx = ptrContainer.size();

        auto newPtr = ptrMakerFunc(desc.filepath);
        if (!newPtr)
        {
            return MAKE_ERROR(Mix_GetError());
        }

        ptrContainer.emplace_back(std::move(newPtr));

        //bool needRepopulateViews = audioInfo_.Size() == audioInfo_.Capacity();
        //if (needRepopulateViews)
        //{
        //    const size_t newSize = audioInfo_.Size() + kDefaultAudioInfoCapacity;

        //    audioInfo_.Reserve(newSize);
        //    RepopulateAudioNameIndexMap(newSize);
        //}

        const size_t newResourceIdx = audioInfo_.PushBack({
            .audioType = desc.audioType,
            .name = std::move(desc.name),
			.filepath = std::move(desc.filepath),
            .storageIndex = newStorageIdx
		});

        auto [_, inserted] = nameToInfoIdx_.try_emplace(
            audioInfo_.GetView<&AudioInfo::name>(newResourceIdx), newStorageIdx
        );
        assert(inserted);

        Handle<Audio> newHandle = 
            Handle<Audio>::Create(audioBankInstanceId_, newResourceIdx);

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

bool AudioBank::HasAudio(std::string_view name) const
{
    return nameToInfoIdx_.contains(name);
}

Result<SoundInstanceResource> AudioBank::GetSoundInstanceResouce(const Handle<Audio>& handle)
{
    return GetAudioInstanceDataInternal<Mix_Chunk>(handle);
}

Result<MusicInstanceResource> AudioBank::GetMusicInstanceResource(const Handle<Audio>& handle)
{
    return GetAudioInstanceDataInternal<Mix_Music>(handle);
}

Handle<Audio> AudioBank::GetAudio(std::string_view name) const
{
    auto it = nameToInfoIdx_.find(name);
    if (it == nameToInfoIdx_.end())
    {
        return {};
    }

    assert(it->second < audioInfo_.Size());

    return Handle<Audio>::Create(audioBankInstanceId_, it->second);
}

std::vector<AudioDescriptor> AudioBank::ExportAudioDescriptors() const
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

//void AudioBank::RepopulateAudioNameIndexMap(size_t newSize)
//{
//    nameToInfoIdx_.clear();
//    nameToInfoIdx_.reserve(newSize);
//
//    for (size_t i = 0; i < audioInfo_.Size(); ++i)
//    {
//        const auto& name = audioInfo_.GetView<&AudioInfo::name>(i);
//
//        auto [_, inserted] = nameToInfoIdx_.try_emplace(name, i);
//        assert(inserted);
//    }
//}