#pragma once
#include "Handle.h"

struct Audio;
struct SpoofStruct;

template <typename T>
concept SomeResourceType = (std::same_as<T, Audio> || std::same_as<T, SpoofStruct>);

template <typename T>
class Handle;

template <SomeResourceType T>
class Handle<T> : public IHandle<Handle<T>>
{
public:
    friend class IHandle<Handle<T>>;

    Handle() = default;
    constexpr bool operator==(const Handle& rhs) const
    {
        return sourceId_ == rhs.sourceId_ && resourceIndex_ == rhs.resourceIndex_ &&
               generation_ == rhs.generation_;
    }

    constexpr uint32_t GetSourceId() const noexcept { return sourceId_; }
    constexpr size_t GetResourceIndex() const noexcept { return static_cast<size_t>(resourceIndex_); }
    constexpr uint32_t GetGeneration() const noexcept { return generation_; }

private:
    size_t GetHashImpl() const noexcept 
    {
        return MakeHash(sourceId_, resourceIndex_, generation_);
    }
    bool IsValidImpl() const
    {
        return sourceId_ != std::numeric_limits<uint32_t>::max() &&
               resourceIndex_ != std::numeric_limits<uint32_t>::max() &&
               generation_ != std::numeric_limits<uint32_t>::max();
    }

    static Handle CreateImpl(uint32_t sourceId, size_t resourceIdx)
    {
        return Handle{ sourceId, resourceIdx };
    }
    static Handle CreateImpl(uint32_t sourceId, size_t resourceIdx, uint32_t gen)
    {
        return Handle{ sourceId, resourceIdx, gen };
    }

    Handle(uint32_t sourceId, size_t resourceIdx) :
        sourceId_(sourceId), resourceIndex_(static_cast<uint32_t>(resourceIdx))
    {
        assert(resourceIdx < static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
    }

    Handle(uint32_t sourceId, size_t resourceIdx, uint32_t gen) :
        sourceId_(sourceId), resourceIndex_(static_cast<uint32_t>(resourceIdx)),
        generation_(gen)
    {
        assert(resourceIdx < static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
    }

    uint32_t sourceId_      = std::numeric_limits<uint32_t>::max();
    uint32_t resourceIndex_ = std::numeric_limits<uint32_t>::max();
    uint32_t generation_    = std::numeric_limits<uint32_t>::max();
};

//template <typename T, typename HandleGenerator>
//    requires (std::is_invocable_r_v<Handle<T>, HandleGenerator, size_t>)
//struct HandleIterator
//{
//	using iterator_category = std::forward_iterator_tag;
//	using value_type = Handle<T>;
//	using difference_type = std::ptrdiff_t;
//	using reference = value_type;
//	using pointer = void;
//
//    constexpr HandleIterator(HandleGenerator handleGen = {}, std::size_t idx = 0)
//		: handleGenerator_(std::move(handleGen)), idx_(idx) {}
//
//	constexpr value_type operator*() const
//	{
//        if (handleGenerator_)
//        {
//            return std::invoke(handleGenerator_, idx_);
//        }
//        return Handle<T>{};
//	}
//
//	constexpr HandleIterator& operator++()
//	{
//		++idx_;
//		return *this;
//	}
//	constexpr HandleIterator operator++(int)
//	{
//        HandleIterator tmp = *this;
//		++(*this);
//		return tmp;
//	}
//
//	constexpr bool operator==(HandleIterator const& other) const
//	{
//		return handleGenerator_ == other.handleGenerator_ && idx_ == other.idx_;
//	}
//	constexpr bool operator!=(HandleIterator const& other) const
//	{
//		return !(*this == other);
//	}
//
//private:
//    HandleGenerator handleGenerator_;
//	size_t idx_;
//};