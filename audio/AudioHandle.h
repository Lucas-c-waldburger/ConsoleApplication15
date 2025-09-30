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