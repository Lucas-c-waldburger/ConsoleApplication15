#pragma once
#include "Atlas.h"
#include "../core/Handle.h"

//struct NewTextureAtlas;
//
//template <>
//class Handle<NewTextureAtlas> : public IHandle<Handle<NewTextureAtlas>>
//{
//public:
//    friend class Super;
//
//    Handle() = default;
//    bool operator==(const Handle& rhs) const
//    {
//        return id_ == rhs.id_ && textureType_ == rhs.textureType_;
//    }
//
//    TextureType GetTextureType() const { return textureType_; }
//
//private:
//    static inline int idCount = 0;
//
//    size_t GetHashImpl() const noexcept { return std::hash<int>{}(id_); }
//    bool IsValidImpl() const
//    {
//        return id_ > -1 && textureType_ != TextureType::Unknown;
//    }
//
//    static Handle CreateImpl(TextureType type)
//    {
//        return Handle{ idCount++, type };
//    }
//
//    Handle(int id, TextureType type) : id_(id), textureType_(type) {}
//
//    int id_ = -1;
//    TextureType textureType_ = TextureType::Unknown;
//};