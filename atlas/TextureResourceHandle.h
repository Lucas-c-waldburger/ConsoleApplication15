#pragma once
#include "../core/Handle.h"
#include <cassert>
#include <limits>

//struct TextureAtlasID { 
//    uint32_t value = std::numeric_limits<uint32_t>::max(); 
//    friend constexpr bool operator==(TextureAtlasID lhs, TextureAtlasID rhs) {
//        return lhs.value == rhs.value;
//    }
//};
//struct TextureResourceIndex {
//    uint32_t value = std::numeric_limits<uint32_t>::max();
//    friend constexpr bool operator==(TextureResourceIndex lhs, TextureResourceIndex rhs) {
//        return lhs.value == rhs.value;
//    }
//};
//
//namespace std {
//template <>
//struct hash<TextureAtlasID> {
//    size_t operator()(TextureAtlasID id) const noexcept {
//        return std::hash<uint32_t>{}(id.value);
//    }
//};
//template <>
//struct hash<TextureResourceIndex> {
//    size_t operator()(TextureResourceIndex idx) const noexcept {
//        return std::hash<uint32_t>{}(idx.value);
//    }
//};
//} // std

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
    constexpr bool operator==(const Handle& rhs) const noexcept
    {
        return id_ == rhs.id_;
    }

    TextureAtlasID GetAtlasID() const 
    { 
        return ExtractTextureAtlasID(id_); 
    }
    TextureResourceIndex GetResourceIndex() const 
    { 
        return ExtractTextureResourceIndex(id_); 
    }

private:
    size_t GetHashImpl() const noexcept
    {
        return std::hash<TextureResourceID>{}(id_);
    }
    bool IsValidImpl() const
    {
        return id_ != kInvalidTextureResourceID;
    }

    static Handle CreateImpl(TextureAtlasID atlasId, size_t resourceIndex)
    {
        return Handle{ atlasId, resourceIndex };
    }

    Handle(TextureAtlasID atlasId, size_t resourceIndex) : 
        id_(ComposeTextureResourceID(atlasId, { static_cast<uint32_t>(resourceIndex) }))
    {
        assert(IndexInTextureResourceRange(resourceIndex));
    }

    TextureResourceID id_ = kInvalidTextureResourceID;
};

struct AtlasResourceHeteroHash
{
    using is_transparent = void;

    size_t operator()(TextureAtlasID atlasId) const noexcept {
        return std::hash<TextureAtlasID>{}(atlasId);
    }
    size_t operator()(const Handle<TextureResource>& resourceHandle) const noexcept {
        return std::hash<TextureAtlasID>{}(resourceHandle.GetAtlasID());
    }
};

struct AtlasResourceHeteroEq
{
    using is_transparent = void;

    bool operator()(TextureAtlasID lhs, TextureAtlasID rhs) const {
        return lhs == rhs;
    }
    bool operator()(const Handle<TextureResource>& resourceHandle,
                    TextureAtlasID atlasId) const {
        return resourceHandle.GetAtlasID() == atlasId;
    }
};

template <typename Value, typename...Args>
using UnorderedAtlasIDMap = 
    std::unordered_map<TextureAtlasID, Value, AtlasResourceHeteroHash,
                                              AtlasResourceHeteroEq, Args...>;