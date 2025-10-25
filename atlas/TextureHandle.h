#pragma once
#include "NewAtlasHandle.h"
 

struct Texture;
using TextureHandle = Handle<Texture>;

template <>
class Handle<Texture> : public IHandle<Handle<Texture>>
{
public:
    friend class Super;

    Handle() = default;
    bool operator==(const Handle& rhs) const
    {
        return sourceAtlasHandle_ == rhs.sourceAtlasHandle_ && plotIndex_ == rhs.plotIndex_;
    }

    TextureType GetTextureType() const { return sourceAtlasHandle_.GetTextureType(); }
    int GetPlotIndex() const { return plotIndex_; }

private:
    size_t GetHashImpl() const noexcept 
    { 
        size_t hash = 0;
        HashCombine(hash, sourceAtlasHandle_.GetHash());
        HashCombine(hash, std::hash<int>{}(plotIndex_));

        return hash;
    }
    bool IsValidImpl() const
    {
        return sourceAtlasHandle_.IsValid() && plotIndex_ > -1;
    }

    static Handle CreateImpl(const Handle<NewTextureAtlas>& srcAtlas, int plotIdx)
    {
        return Handle{ srcAtlas, plotIdx };
    }

    Handle(const Handle<NewTextureAtlas>& srcAtlas, int plotIdx) : 
        sourceAtlasHandle_(srcAtlas), plotIndex_(plotIdx) {}

    Handle<NewTextureAtlas> sourceAtlasHandle_;
    int plotIndex_ = -1;
};