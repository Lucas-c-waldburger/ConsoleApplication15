#pragma once
#include <unordered_map>
#include <vector>
#include <cassert>
#include <unordered_set>
#include "../core/Result.h"
#include "../core/ResourceHandle.h"
#include "AudioInstance.h"
#include "AudioDescriptor.h"
#include "../core/StableSOA.h"
#include "../core/Dictionary.h"

//// TODO: MAKE SURE AUDIO SYSTEM / MANAGER CAN HANDLE ERASEAUDIO!!
struct AudioInfo
{
    AudioType audioType = AudioType::Unknown;
    std::string name;
    std::string filepath;
    uint32_t length;
    size_t storageIndex = std::numeric_limits<size_t>::max();
    uint32_t generation = 0;
};

using AudioInfoSOA = StableSOA<
    AudioInfo,
	&AudioInfo::audioType,
	&AudioInfo::name,
	&AudioInfo::filepath,
    &AudioInfo::length,
    &AudioInfo::storageIndex,
    &AudioInfo::generation
>;

class AudioBank
{
public:
    friend class AudioSystem;

    static constexpr size_t kDefaultAudioInfoCapacity = 40;

	AudioBank() : audioBankInstanceId_(audioBankInstanceIdCounter_++) { 
        audioInfo_.Reserve(kDefaultAudioInfoCapacity);
    }
	~AudioBank() = default;

	AudioBank(const AudioBank&) = delete;
	AudioBank& operator=(const AudioBank&) = delete;
    AudioBank(AudioBank&&) noexcept = default;
    AudioBank& operator=(AudioBank&&) noexcept = default;

    Handle<Audio> GetAudio(std::string_view name) const;
	Result<Handle<Audio>> LoadAudio(AudioDescriptor&& desc);
	bool HasAudio(std::string_view name) const;

    bool IsAudioValid(const Handle<Audio>& handle) const;

    bool EraseAudio(const Handle<Audio>& handle);

    Result<Void> SetAudioName(const Handle<Audio>& handle, std::string_view newName);

    auto GetAudioInfo(const Handle<Audio>& handle) const
    {
        using Ret = decltype(audioInfo_.TryGetView(0));

        if (!IsAudioValid(handle))
        {
            return Ret{ std::nullopt };
        }

        const auto& cInfo = audioInfo_;

        return cInfo.TryGetView(handle.GetResourceIndex());
    }

    template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 1)
    auto GetAudioInfo(const Handle<Audio>& handle) const
    {
		const size_t resourceIdx = (handle.GetSourceId() == audioBankInstanceId_) 
            ? handle.GetResourceIndex() 
			: std::numeric_limits<size_t>::max();

        return GetAudioInfoImpl<MemberPtrs...>(resourceIdx);
    }

    template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 1)
    auto GetAudioInfo(std::string_view name) const
    {
		auto it = nameToInfoIdx_.find(name);

		return GetAudioInfoImpl<MemberPtrs...>(it != nameToInfoIdx_.end() 
            ? it->second 
            : std::numeric_limits<size_t>::max());
    }

    template <auto MemberPtr>
    auto GetAudioInfo(const Handle<Audio>& handle) const
    {
        const size_t resourceIdx = (handle.GetSourceId() == audioBankInstanceId_)
            ? handle.GetResourceIndex()
            : std::numeric_limits<size_t>::max();

        return GetAudioInfoImpl<MemberPtr>(resourceIdx);
    }

    template <auto MemberPtr>
    auto GetAudioInfo(std::string_view name) const
    {
        auto it = nameToInfoIdx_.find(name);

        return GetAudioInfoImpl<MemberPtr>(it != nameToInfoIdx_.end()
            ? it->second
            : std::numeric_limits<size_t>::max());
    }

    template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
    auto IterAudioInfo() const
    {
        return audioInfo_.ForEach<MemberPtrs...>();
    }

	std::vector<AudioDescriptor> ExportAudioDescriptors() const;

    size_t GetAudioInfoSize() const { return audioInfo_.Size(); }
    size_t GetAudioInfoCapacity() const { return audioInfo_.Capacity(); }

    size_t GetSoundCount() const;
    size_t GetMusicCount() const;

    uint32_t GetBankID() const noexcept { return audioBankInstanceId_; }

    template <SomeMixType T>
    T* GetAudioPtr(const Handle<Audio>& handle)
    {
        if (!IsAudioValid(handle))
        {
            return nullptr;
        }

        const auto [audioType, storageIdx] = audioInfo_.GetView<&AudioInfo::audioType, 
                                                                &AudioInfo::storageIndex>
                                                                (handle.GetResourceIndex());
        if constexpr (std::same_as<T, Mix_Music>)
        {
            if (audioType != AudioType::Music)
            {
                return nullptr;
            }
            assert(storageIdx < music_.size());

            return music_[storageIdx].get();
        }
        else
        {
            if (audioType != AudioType::Sound)
            {
                return nullptr;
            }
            assert(storageIdx < sounds_.size());

            return sounds_[storageIdx].get();
        }
    }

    template <SomeMixType T>
    T* GetAudioPtr(const Handle<Audio>& handle) const
    {
        if (!IsAudioValid(handle))
        {
            return nullptr;
        }

        const auto [audioType, storageIdx] = audioInfo_.GetView<&AudioInfo::audioType,
                                                                &AudioInfo::storageIndex>
                                                                (handle.GetResourceIndex());
        if constexpr (std::same_as<T, Mix_Music>)
        {
            if (audioType != AudioType::Music)
            {
                return nullptr;
            }
            assert(storageIdx < music_.size());

            return music_[storageIdx].get();
        }
        else
        {
            if (audioType != AudioType::Sound)
            {
                return nullptr;
            }
            assert(storageIdx < sounds_.size());

            return sounds_[storageIdx].get();
        }
    }


    Result<SoundInstanceResource> GetSoundInstanceResouce(const Handle<Audio>& handle);
    Result<MusicInstanceResource> GetMusicInstanceResource(const Handle<Audio>& handle);

private:
    template <typename T> requires (std::same_as<T, Mix_Chunk> ||
                                    std::same_as<T, Mix_Music>)
    Result<AudioInstanceResource<T>> GetAudioInstanceDataInternal(const Handle<Audio>& handle);

    template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 1)
    auto GetAudioInfoImpl(size_t resourceIdx) const
    {
		using Ret = decltype(audioInfo_.TryGetView<MemberPtrs...>(0));

        if (resourceIdx >= audioInfo_.Size())
        {
            return Ret{ std::nullopt };
        }

        const auto& cInfo = audioInfo_;

        return cInfo.TryGetView<MemberPtrs...>(resourceIdx);
    }

    template <auto MemberPtr>
    auto GetAudioInfoImpl(size_t resourceIdx) const
    {
        using Ret = MonoValueOptionalTupleUnwrapper<
            const typename member_ptr_traits<MemberPtr>::value_type&>;

        if (resourceIdx >= audioInfo_.Size())
        {
            return Ret{};
        }

        const auto& cInfo = audioInfo_;

        return Ret{ cInfo.TryGetView<MemberPtr>(resourceIdx) };
    }

	static inline uint32_t audioBankInstanceIdCounter_ = 0;

    uint32_t audioBankInstanceId_ = 0;

    std::vector<SoundPtr> sounds_;
    std::vector<MusicPtr> music_;
    AudioInfoSOA audioInfo_;
    UnorderedDictionary<size_t> nameToInfoIdx_;

    std::vector<size_t> freeSoundSlots_;
    std::vector<size_t> freeMusicSlots_;
    //std::unordered_set<std::uintptr_t> staleAudioPointers_;
};

template <typename T> requires (std::same_as<T, Mix_Chunk> ||
                                std::same_as<T, Mix_Music>)
inline Result<AudioInstanceResource<T>>
AudioBank::GetAudioInstanceDataInternal(const Handle<Audio>& handle)
{
    if (handle.GetSourceId() != audioBankInstanceId_)
    {
        return MAKE_ERROR("Audio handle did not belong to this audio bank instance");
	}

    const size_t resourceIdx = handle.GetResourceIndex();
    if (resourceIdx >= audioInfo_.Size())
    {
        return MAKE_ERROR("Audio handle's resource index was out of range");
	};

	const auto& [audioType, storageIdx] = 
        audioInfo_.GetView<&AudioInfo::audioType, &AudioInfo::storageIndex>(resourceIdx);

    auto makeInstanceData = [&](auto& ptrContainer, AudioType expectedType) 
    -> Result<AudioInstanceResource<T>> {
        if (audioType != expectedType)
        {
            return MAKE_ERROR("Audio handle did not have expected audio type");
        }
        if (storageIdx >= ptrContainer.size())
        {
            return MAKE_ERROR("Storage index out of range");
        }

		auto* audioPtr = ptrContainer[storageIdx].get();
        assert(audioPtr);

        return AudioInstanceResource<T>{
            .audioPtr = audioPtr,
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