#pragma once
#include "AudioCommon.h"
#include "../core/Handle.h"

struct Audio;

template <>
class Handle<Audio> : public IHandle<Handle<Audio>>
{
public:
    friend class Super;

    Handle() = default;
    bool operator==(const Handle& rhs) const
    {
        return id_ == rhs.id_ && audioType_ == rhs.audioType_;
    }

    AudioType GetAudioType() const { return audioType_; }

private:
    static inline int idCount = 0;

    size_t GetHashImpl() const noexcept { return std::hash<int>{}(id_); }
    bool IsValidImpl() const
    {
        return id_ > -1 && audioType_ != AudioType::Unknown;
    }

    static Handle CreateImpl(AudioType type)
    {
        return Handle{ idCount++, type };
    }

    Handle(int id, AudioType type) : id_(id), audioType_(type) {}

    int id_ = -1;
    AudioType audioType_ = AudioType::Unknown;
};

//struct Audio2;
//
//template <>
//class Handle<Audio2> : public IHandle<Handle<Audio2>>
//{
//public:
//    friend class IHandle<Handle<Audio2>>;
//
//    Handle() = default;
//    bool operator==(const Handle& rhs) const
//    {
//        return sourceId_ == rhs.sourceId_ && resourceIndex_ == rhs.resourceIndex_;
//    }
//
//    constexpr uint32_t GetSourceId() const noexcept { return sourceId_; }
//    constexpr size_t GetResourceIndex() const noexcept { return static_cast<size_t>(resourceIndex_); }
//
//private:
//    size_t GetHashImpl() const noexcept
//    {
//        return MakeHash(sourceId_, resourceIndex_);
//    }
//    bool IsValidImpl() const
//    {
//        return sourceId_ != std::numeric_limits<uint32_t>::max() &&
//               resourceIndex_ != std::numeric_limits<uint32_t>::max();
//    }
//
//    static Handle CreateImpl(uint32_t sourceId, size_t resourceIdx)
//    {
//        return Handle{ sourceId, resourceIdx };
//    }
//
//    Handle(uint32_t sourceId, size_t resourceIdx) :
//        sourceId_(sourceId), resourceIndex_(static_cast<uint32_t>(resourceIdx))
//    {
//        assert(resourceIdx < static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
//    }
//
//    uint32_t sourceId_ = std::numeric_limits<uint32_t>::max();
//    uint32_t resourceIndex_ = std::numeric_limits<uint32_t>::max();
//};

//struct Audio2;
//
//template <> 
//struct is_resource_type<Audio2> : std::true_type {};

//template <>
//class Handle<Audio2> : public IHandle<Handle<Audio2>>
//{
//public:
//    friend class Super;
//
//    Handle() = default;
//    bool operator==(const Handle& rhs) const
//    {
//        return bankInstanceId_ == rhs.bankInstanceId_ && 
//               resourceIndex_ == rhs.resourceIndex_;
//    }
//
//	constexpr uint32_t GetBankInstanceId() const noexcept { return bankInstanceId_; }
//	constexpr size_t GetResourceIndex() const noexcept { return static_cast<size_t>(resourceIndex_); }
//
//private:
//    size_t GetHashImpl() const noexcept { 
//		return MakeHash(bankInstanceId_, resourceIndex_);
//    }
//    bool IsValidImpl() const
//    {
//        return bankInstanceId_ != std::numeric_limits<uint32_t>::max() &&
//               resourceIndex_ != std::numeric_limits<uint32_t>::max();
//    }
//
//    static Handle CreateImpl(uint32_t bankInstanceId, size_t resourceIdx)
//    {
//        return Handle{ bankInstanceId, resourceIdx };
//    }
//
//    Handle(uint32_t bankInstanceId, size_t resourceIdx) : 
//        bankInstanceId_(bankInstanceId), resourceIndex_(static_cast<uint32_t>(resourceIdx)) 
//    {
//		assert(resourceIdx < static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
//    }
//
//	uint32_t bankInstanceId_ = std::numeric_limits<uint32_t>::max();
//	uint32_t resourceIndex_ = std::numeric_limits<uint32_t>::max();
//};