#pragma once
#include <unordered_map>
#include "SpriteAnimationSeries.h"
#include "../core/TransparentStringHash.h"

class SpriteAnimationSeriesMap
{
public:
    using MapType = std::unordered_map<std::string, SpriteAnimationSeries,
                                       TransparentStringHash, std::equal_to<>>;
    using EmplaceRetType = std::pair<typename MapType::iterator, bool>;

    // map operations
    EmplaceRetType Emplace(std::string_view seriesName, const SpriteAnimationSeries& series);
    EmplaceRetType Emplace(std::string_view seriesName, SpriteAnimationSeries&& series);

    bool Erase(std::string_view seriesName);

    bool Contains(std::string_view seriesName) const;

    // current series operations
    const SpriteAnimationSeries* GetCurrent() const;

    bool HasCurrent() const;

    bool SetCurrent(std::string_view seriesName, bool resetOriginal);
    bool SetCurrent(std::string_view seriesName);

    bool SetCurrentIndex(size_t newIndex);

    // increment current series sprite. true means we have a new sprite to render
    bool NextInSeries();

    // update operations
    bool NeedsUpdate() const { return dirty_; }
    void MarkUpdated() { dirty_ = false; }

private:
    MapType map_;
    std::string_view current_;
    bool dirty_ = true;
};
