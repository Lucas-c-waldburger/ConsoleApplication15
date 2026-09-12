#include "AudioBank.h"
#include <filesystem>

uint32_t GetAudioLength(const MusicPtr& musicPtr)
{
    const double len = Mix_MusicDuration(musicPtr.get());
    if (len < 0.0)
    {
        return 0;
    }

    return static_cast<uint32_t>(len * 1000.0);
}

uint32_t GetAudioLength(const SoundPtr& soundPtr)
{
    if (!soundPtr)
    {
        return 0;
    }

    int freq;
    int channels;
    Uint16 format;

    if (Mix_QuerySpec(&freq, &format, &channels) == 0)
    {
        return 0; // sound not open
    }

    if (format == 0 || channels == 0)
    {
        return 0;
    }

    int sampleSize = SDL_AUDIO_BITSIZE(format) / 8;

    Uint32 points = soundPtr->alen / sampleSize;
    Uint32 frames = points / channels;

    return (frames * 1000) / freq;
}

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

            auto [audioType, name, filepath, len, storageIdx, gen] =
                audioInfo_.GetView(infoIdx);

            assert(audioType != AudioType::Unknown);
            assert(name.empty());
            assert(!nameToInfoIdx_.contains(name));
            assert(filepath.empty());
            assert(gen > 0);
            assert(storageIdx < ptrContainer.size());
            assert(!ptrContainer[storageIdx]);

            name = desc.name;
            filepath = std::move(desc.filepath);
            len = GetAudioLength(newPtr);

            ptrContainer[storageIdx] = std::move(newPtr);

            [[maybe_unused]] auto [_, nameInserted] = 
                nameToInfoIdx_.try_emplace(std::move(name), infoIdx);
            assert(nameInserted);

            freeSlotContainer.pop_back();

            return Handle<Audio>::Create(audioBankInstanceId_, infoIdx, gen);
        }
        else
        {
            const size_t newStorageIdx = ptrContainer.size();

            const size_t newResourceIdx = audioInfo_.PushBack({
                .audioType = desc.audioType,
                .name = desc.name,
                .filepath = std::move(desc.filepath),
                .length = GetAudioLength(newPtr),
                .storageIndex = newStorageIdx,
                .generation = 0
             });

            ptrContainer.emplace_back(std::move(newPtr));

            [[maybe_unused]] auto [_, nameInserted] = 
                nameToInfoIdx_.try_emplace(std::move(desc.name), newStorageIdx);
            assert(nameInserted);

            return Handle<Audio>::Create(audioBankInstanceId_, newResourceIdx, 0);
        }
    };

    switch (desc.audioType)
    {
    case AudioType::Sound:
        return impl(freeSoundSlots_, sounds_, MakeSoundPtr);
    case AudioType::Music:
        return impl(freeMusicSlots_, music_, MakeMusicPtr);
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
    if (handle.GetSourceId() != audioBankInstanceId_ ||
        handle.GetResourceIndex() >= audioInfo_.Size())
    {
        return false;
    }

    const auto [audioType, gen, storageIdx] = audioInfo_.GetView<&AudioInfo::audioType,
                                                                 &AudioInfo::generation,
                                                                 &AudioInfo::storageIndex>
                                                                 (handle.GetResourceIndex());

    return gen == handle.GetGeneration() && (audioType == AudioType::Music)
        ? (storageIdx < music_.size() && music_[storageIdx])
        : (storageIdx < sounds_.size() && sounds_[storageIdx]);
}

bool AudioBank::EraseAudio(const Handle<Audio>& handle)
{
    if (!IsAudioValid(handle))
    {
        return false;
    }

    auto [audioType, name, filepath, duration, storageIdx, gen] = 
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

    [[maybe_unused]] const size_t nameErased = nameToInfoIdx_.erase(name);
    assert(nameErased > 0);

    name.clear();
    filepath.clear();
    duration = 0;
    ++gen;

    (audioType == AudioType::Music)
        ? music_[storageIdx].reset()
        : sounds_[storageIdx].reset();

    freeSlots.emplace_back(handle.GetResourceIndex());

    return true;
}

Result<Void> AudioBank::SetAudioName(const Handle<Audio>& handle, std::string_view newName)
{
    if (!IsAudioValid(handle))
    {
        return MAKE_ERROR("Audio handle was invalid");
    }
    if (nameToInfoIdx_.contains(newName))
    {
        return MAKE_ERROR_FMT("Duplicate audio name '{}' already exists in bank", newName);
    }

    auto& oldName = audioInfo_.GetView<&AudioInfo::name>(handle.GetResourceIndex());

    [[maybe_unused]] const size_t nameErased = nameToInfoIdx_.erase(oldName);
    assert(nameErased > 0);

    oldName = newName;

    nameToInfoIdx_.try_emplace(newName, handle.GetResourceIndex());

    return kVoid;
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

size_t AudioBank::GetSoundCount() const
{
    assert(sounds_.size() >= freeSoundSlots_.size());
    return sounds_.size() - freeSoundSlots_.size();
}

size_t AudioBank::GetMusicCount() const
{
    assert(music_.size() >= freeMusicSlots_.size());
    return music_.size() - freeMusicSlots_.size();
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