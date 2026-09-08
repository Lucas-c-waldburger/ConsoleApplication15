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

    auto impl = [this, &desc](auto& freeSlotContainer, auto& ptrContainer, auto& ptrMakerFunc) 
    -> Result<Handle<Audio>> {

        auto newPtr = ptrMakerFunc(desc.filepath);
        if (!newPtr)
        {
            return MAKE_ERROR(Mix_GetError());
        }

        if (!freeSlotContainer.empty())
        {
            const size_t infoIdx = freeSlotContainer.back();
            assert(infoIdx < audioInfo_.Size());

            auto [audioType, name, filepath, storageIdx, gen] =
                audioInfo_.GetView(infoIdx);

            assert(audioType != AudioType::Unknown);
            assert(name.empty());
            assert(!nameToInfoIdx_.contains(name));
            assert(filepath.empty());
            assert(gen > 0);
            assert(storageIdx < ptrContainer.size())
            assert(!ptrContainer[storageIdx]);

            name = desc.name;
            filepath = std::move(desc.filepath);

            ptrContainer[newStorageIdx] = std::move(newPtr);

            [[maybe_unused]] auto [_, nameInserted] = 
                nameToInfoIdx_.try_emplace(std::move(name), infoIdx);
            assert(nameInserted);

            freeSlotContainer.pop_back();

            return Handle<Audio>::Create(audioBankInstanceId_, infoIdx, gen);
        }
        else
        {
            const size_t newStorageIdx = ptrContainer.size();

            ptrContainer.emplace_back(std::move(newPtr));

            const size_t newResourceIdx = audioInfo_.PushBack({
                .audioType = desc.audioType,
                .name = desc.name,
                .filepath = std::move(desc.filepath),
                .storageIndex = newStorageIdx,
                .generation = 0
             });

            [[maybe_unused]] auto [_, nameInserted] = 
                nameToInfoIdx_.try_emplace(std::move(desc.name), newStorageIdx);
            assert(nameInserted);

            return Handle<Audio>::Create(audioBankInstanceId_, newResourceIdx, 0);
        }
    };

    switch (desc.audioType)
    {
    case AudioType::Sound:
        return impl(sounds_, MakeSoundPtr, freeSoundSlots_);
    case AudioType::Music:
        return impl(music_, MakeMusicPtr, freeMusicSlots_);
    case AudioType::Unknown: default:
        return MAKE_ERROR("Audio descriptor's audio type was Unknown");
    }
}

bool AudioBank::HasAudio(std::string_view name) const
{
    return nameToInfoIdx_.contains(name);
}

bool AudioBank::IsAudioValid(const Handle<Audio>& handle) const
{
    return handle.GetSourceId() == audioBankInstanceId_ &&
           handle.GetResourceIndex() < audioInfo_.Size() &&
           audioInfo_.GetView<&AudioInfo::generation>(handle.GetResourceIndex()) ==
           handle.GetGeneration();
}

bool AudioBank::EraseAudio(const Handle<Audio>& handle)
{
    if (!IsAudioValid(handle))
    {
        return false;
    }

    auto [audioType, name, filepath, storageIdx, gen] = 
        audioInfo_.GetView(handle.GetResourceIndex());

    assert(audioType != AudioType::Unknown);
    assert((audioType == AudioType::Music && storageIdx < music_.size()) ||
           (storageIdx < sounds_.size()));

    if (filepath.empty())
    {
        assert(name.empty());
        assert((audioType == AudioType::Music && !music_[storageIdx]) ||
               (!sounds_[storageIdx]));

        return false;
    }

    assert((audioType == AudioType::Music && music_[storageIdx]) ||
           (sounds_[storageIdx]));

    auto& freeSlots = (audioType == AudioType::Music)
        ? freeMusicSlots_
        : freeSoundSlots_;

    name.clear();
    filepath.clear();
    ++gen;

    (audioType == AudioType::Music)
        ? music_[storageIdx].reset()
        : sounds_[storageIdx].reset();

    [[maybe_unused]] const size_t nameErased = nameToInfoIdx_.erase(name);
    assert(nameErased > 0);

    freeSlots.emplace_back(handle.GetResourceIndex());

    return true;
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

    return Handle<Audio>::Create(
        audioBankInstanceId_, 
        it->second,
        audioInfo_.GetView<&AudioInfo::generation>(it->second)
    );
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