#pragma once
#include <unordered_map>
#include <vector>
#include <cassert>
#include "../core/Result.h"
#include "AudioHandle.h"
#include "AudioInstance.h"
#include "AudioBank.h"
#include "../core/StableSOA.h"
#include "../core/Dictionary.h"

struct AudioInfo
{
    AudioType audioType = AudioType::Sound;
    std::string name;
    std::string filepath;
    size_t storageIndex = std::numeric_limits<size_t>::max();
};

using AudioInfoSOA = StableSOA<
    AudioInfo,
	&AudioInfo::audioType,
	&AudioInfo::name,
	&AudioInfo::filepath,
    &AudioInfo::storageIndex
>;

class AudioBank2
{
public:
    friend class AudioSystem;

	AudioBank2() : instanceId_(instanceIdCounter_++) {}
	~AudioBank2() = default;

	AudioBank2(const AudioBank2&) = delete;
	AudioBank2& operator=(const AudioBank2&) = delete;
    AudioBank2(AudioBank2&&) = default;
	AudioBank2& operator=(AudioBank2&&) = default;

	Result<Handle<Audio2>> LoadAudio(AudioDescriptor&& desc);
	bool HasAudio(std::string_view name) const;

    template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
    auto GetAudioInfo(const Handle<Audio2>& handle) const
    {
		const size_t resourceIdx = (handle.GetBankInstanceId() == instanceId_) 
            ? handle.GetResourceIndex() 
			: std::numeric_limits<size_t>::max();

        return GetAudioInfoImpl<MemberPtrs...>(resourceIdx);
    }

    template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
    auto GetAudioInfo(std::string_view name) const
    {
		auto it = nameToInfoIdx_.find(name);

		return GetAudioInfoImpl<MemberPtrs...>(it != nameToInfoIdx_.end() 
            ? it->second 
            : std::numeric_limits<size_t>::max());
    }

	std::vector<AudioDescriptor> ExportAudioDescriptors() const;

private:
    Result<SoundInstanceResource> GetSoundInstanceResouce(const Handle<Audio2>& handle);
    Result<MusicInstanceResource> GetMusicInstanceResource(const Handle<Audio2>& handle);

    template <typename T> requires (std::same_as<T, Mix_Chunk> ||
                                    std::same_as<T, Mix_Music>)
    Result<AudioInstanceResource<T>> GetAudioInstanceDataInternal(const Handle<Audio2>& handle);

    template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
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

	static inline uint32_t instanceIdCounter_ = 0;

    uint32_t instanceId_ = 0;

    std::vector<SoundPtr> sounds_;
    std::vector<MusicPtr> music_;
    UnorderedDictionary<size_t> nameToInfoIdx_;
    AudioInfoSOA audioInfo_;
};

template <typename T> requires (std::same_as<T, Mix_Chunk> ||
                                std::same_as<T, Mix_Music>)
inline Result<AudioInstanceResource<T>>
AudioBank2::GetAudioInstanceDataInternal(const Handle<Audio2>& handle)
{
    if (handle.GetBankInstanceId() != instanceId_)
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