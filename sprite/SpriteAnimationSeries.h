#pragma once
#include <algorithm>
#include "../atlas/SpriteSeriesAtlas.h"
#include "../core/Literals.h"
#include "../sdl/SDLUtils.h"

struct SpriteAnimationSeries
{
    Handle<SpriteSeriesAtlas> sourceAtlas;
    std::vector<AtlasPlot> spritePlots;
    size_t index = 0;

    friend bool operator==(const SpriteAnimationSeries& lhs, const SpriteAnimationSeries& rhs)
    {
        return lhs.sourceAtlas == rhs.sourceAtlas && lhs.spritePlots == rhs.spritePlots;
    }
};


//friend void operator++(SpriteAnimationSeries& series)
//{
//    series.index = (series.index + 1) % series.spritePlots.size();
//}
//
//friend void operator++(SpriteAnimationSeries& series, int)
//{
//    series.index = (series.index + 1) % series.spritePlots.size();
//}
//
//friend void operator--(SpriteAnimationSeries& series)
//{
//    size_t newIdx = std::max(0_uz, series.spritePlots.size());
//    series.index = (newIdx == 0) ? 0 : series.spritePlots.size() - 1;
//}
//
//friend void operator--(SpriteAnimationSeries& series, int)
//{
//    size_t newIdx = std::max(0_uz, series.spritePlots.size());
//    series.index = (newIdx == 0) ? 0 : series.spritePlots.size() - 1;
//}