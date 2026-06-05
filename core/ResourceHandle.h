#pragma once
#include "Handle.h"

struct Audio2;
struct SpoofStruct;

template <typename T>
concept SomeResourceType = (std::same_as<T, Audio2> || std::same_as<T, SpoofStruct>);

template <typename T>
class Handle;

template <SomeResourceType T>
class Handle<T> : public IHandle<Handle<T>>
{
public:
    friend class IHandle<Handle<T>>;

    Handle() = default;
    bool operator==(const Handle& rhs) const
    {
        return sourceId_ == rhs.sourceId_ && resourceIndex_ == rhs.resourceIndex_;
    }

    constexpr uint32_t GetSourceId() const noexcept { return sourceId_; }
    constexpr size_t GetResourceIndex() const noexcept { return static_cast<size_t>(resourceIndex_); }

private:
    size_t GetHashImpl() const noexcept 
    {
        return MakeHash(sourceId_, resourceIndex_);
    }
    bool IsValidImpl() const
    {
        return sourceId_ != std::numeric_limits<uint32_t>::max() &&
               resourceIndex_ != std::numeric_limits<uint32_t>::max();
    }

    static Handle CreateImpl(uint32_t sourceId, size_t resourceIdx)
    {
        return Handle{ sourceId, resourceIdx };
    }

    Handle(uint32_t sourceId, size_t resourceIdx) :
        sourceId_(sourceId), resourceIndex_(static_cast<uint32_t>(resourceIdx))
    {
        assert(resourceIdx < static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
    }

    uint32_t sourceId_      = std::numeric_limits<uint32_t>::max();
    uint32_t resourceIndex_ = std::numeric_limits<uint32_t>::max();
};