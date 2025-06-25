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

    EmplaceRetType Emplace(std::string_view seriesName, const SpriteAnimationSeries& series);
    EmplaceRetType Emplace(std::string_view seriesName, SpriteAnimationSeries&& series);

    bool Erase(std::string_view seriesName);

    SpriteAnimationSeries* GetCurrent();
    const SpriteAnimationSeries* GetCurrent() const;
    bool SetCurrent(std::string_view seriesName, bool resetOriginal = true);
    bool HasCurrent() const;
    void Next() { if (HasCurrent()) { ++(*GetCurrent()); } }

    SpriteAnimationSeries* operator[](std::string_view seriesName);
    const SpriteAnimationSeries* operator[](std::string_view seriesName) const;

    bool Contains(std::string_view seriesName) const;

private:
    MapType map_;
    std::string_view current_;
};
