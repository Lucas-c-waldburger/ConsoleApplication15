#pragma once
#include <unordered_map>
#include "SpriteAnimationSeries.h"
#include "../core/TransparentStringHash.h"

//// TODO: think about moving the dirty state logic into a tag component
//class SpriteAnimationsTable
//{
//public:
//    enum ResetOption : uint8_t
//    {
//        None = 0,
//        Index = 1 << 0,
//        SpriteRange = 1 << 1,
//        All = 0xFF
//    };
//
//    using MapType = std::unordered_map<std::string, SpriteAnimationSeries,
//                                       TransparentStringHash, std::equal_to<>>;
//    using EmplaceRetType = std::pair<typename MapType::iterator, bool>;
//
//    // map operations
//    EmplaceRetType Emplace(std::string_view seriesName, const SpriteAnimationSeries& series);
//    EmplaceRetType Emplace(std::string_view seriesName, SpriteAnimationSeries&& series);
//
//    bool Erase(std::string_view seriesName);
//
//    bool Contains(std::string_view seriesName) const;
//
//    // current series operations
//    const SpriteAnimationSeries* GetCurrentSeries() const;
//
//    bool HasCurrentSeries() const;
//
//    // by default, setting a new current will reset index to 0 and set sprite range to the full series range
//    bool SetCurrentSeries(std::string_view seriesName, ResetOption resetOptions);
//    bool SetCurrentSeries(std::string_view seriesName);
//
//    bool SetCurrentIndex(size_t newIndex);
//
//    // define a sub-index range to be advanced through instead of the full sequence
//    bool SetCurrentSpriteRange(Range<size_t> newRange, bool bumpIndexNow);
//    bool SetCurrentSpriteRange(Range<size_t> newRange);
//    void ResetCurrentSpriteRange();
//
//    // increment current series sprite. true means we have a new sprite to render
//    bool NextInSeries();
//
//    // update operations
//    bool NeedsUpdate() const { return dirty_; }
//    void MarkUpdated() { dirty_ = false; }
//
//private:
//    MapType map_;
//    std::string_view current_;
//    bool dirty_ = true;
//};
