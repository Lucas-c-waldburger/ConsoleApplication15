#pragma once
#include "../core/Handle.h"
#include <cassert>
#include <limits>

using TextureAtlasID = uint32_t;
using TextureResourceIndex = uint32_t;
using TextureResourceID = uint64_t;

inline constexpr TextureAtlasID 
ExtractTextureAtlasID(TextureResourceID resourceId)
{
    return static_cast<uint32_t>(resourceId >> 32);
}

inline constexpr TextureResourceIndex 
ExtractTextureResourceIndex(TextureResourceID resourceId)
{
    return static_cast<uint32_t>(resourceId & 0xFFFFFFFFull);
}

inline constexpr TextureResourceID
ComposeTextureResourceID(TextureAtlasID atlasId, TextureResourceIndex resourceIdx)
{
    return ((static_cast<uint64_t>(atlasId) << 32ull) | 
             static_cast<uint64_t>(resourceIdx));
}

inline constexpr std::pair<TextureAtlasID, TextureResourceIndex>
DecomposeTextureResourceID(TextureResourceID resourceId)
{
    return std::make_pair(ExtractTextureAtlasID(resourceId), 
                          ExtractTextureResourceIndex(resourceId));
}

inline constexpr bool IndexInTextureResourceRange(size_t idx)
{
    return idx <= static_cast<size_t>(std::numeric_limits<uint32_t>::max());
}

inline constexpr TextureAtlasID kInvalidTextureAtlasID =
    std::numeric_limits<TextureAtlasID>::max();

inline constexpr TextureResourceIndex kInvalidTextureResourceIndex =
    std::numeric_limits<TextureResourceIndex>::max();

inline constexpr TextureResourceID kInvalidTextureResourceID =
    std::numeric_limits<TextureResourceID>::max();

class TextureResource { TextureResource() = default; };

template <>
class Handle<TextureResource> : public IHandle<Handle<TextureResource>>
{
public: 
    friend class Super;

    Handle() = default;
    constexpr bool operator==(const Handle<TextureResource>& rhs) const noexcept
    {
        return atlasId_ == rhs.atlasId_ && resourceIndex_ == rhs.resourceIndex_ &&
               generation_ == rhs.generation_;
    }

    // The atlas texture the resource is sourced from
    constexpr uint32_t GetAtlasID() const noexcept
    { 
        return atlasId_;
    }
    // The index of the resource within the info SOA
    constexpr uint32_t GetResourceIndex() const noexcept
    { 
        return resourceIndex_;
    }
	// The generation of the resource - only used by SpriteAtlasCollection for reusing plots
    constexpr uint32_t GetGeneration() const noexcept
    {
        return generation_;
    }

private:
    size_t GetHashImpl() const noexcept
    {
        return MakeHash(atlasId_, resourceIndex_, generation_);
    }
    bool IsValidImpl() const
    {
        return atlasId_ != std::numeric_limits<uint32_t>::max() &&
               resourceIndex_ != std::numeric_limits<uint32_t>::max();
    }

    static Handle CreateImpl(TextureAtlasID atlasId, size_t resourceIndex)
    {
        return Handle{ atlasId, resourceIndex };
    }
    static Handle CreateImpl(TextureAtlasID atlasId, size_t resourceIndex, uint32_t gen)
    {
        return Handle{ atlasId, resourceIndex, gen };
    }

    Handle(TextureAtlasID atlasId, size_t resourceIndex) : 
        atlasId_(atlasId), resourceIndex_(resourceIndex)
    {
        assert(IndexInTextureResourceRange(resourceIndex));
    }

    Handle(TextureAtlasID atlasId, size_t resourceIndex, uint32_t gen) :
        atlasId_(atlasId), resourceIndex_(resourceIndex), generation_(gen)
    {
        assert(IndexInTextureResourceRange(resourceIndex));
    }

    uint32_t atlasId_ = std::numeric_limits<size_t>::max();
    uint32_t resourceIndex_ = std::numeric_limits<size_t>::max();
    uint32_t generation_ = 0;
};