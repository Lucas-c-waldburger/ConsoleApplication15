#include "SpriteAnimationSeriesMap.h"
//#include "../core/Literals.h"
//#include <algorithm>
//
//namespace {
//
//void ClampSpriteRangeToSeriesSize(Range<size_t>& spriteRange, const SpriteAnimationSeries& series)
//{
//    spriteRange.min = std::clamp(spriteRange.min, 0_uz, series.spritePlots.size());
//    spriteRange.max = std::clamp(spriteRange.max, 0_uz, series.spritePlots.size());
//}
//
//void AdvanceSeriesIndex(SpriteAnimationSeries& series)
//{
//    assert(series.spriteRange.min <= series.spriteRange.max);
//
//    series.index = (series.index + 1 > series.spriteRange.max)
//        ? series.spriteRange.min
//        : series.index + 1;
//}
//
//constexpr bool IndexInSpriteRange(size_t index, const Range<size_t>& spriteRange)
//{
//    return index >= spriteRange.min && index <= spriteRange.max;
//}
//
//} // unnamed
//
//SpriteAnimationsTable::EmplaceRetType
//SpriteAnimationsTable::Emplace(std::string_view seriesName, const SpriteAnimationSeries& series)
//{
//    auto result = map_.emplace(std::string{seriesName}, series);
//    if (result.second)
//    {
//        ClampSpriteRangeToSeriesSize(result.first->second.spriteRange, result.first->second);
//
//        if (map_.size() == 1)
//        {
//            current_ = result.first->first;
//            dirty_ = true;
//        }
//    }
//
//    return result;
//}
//
//SpriteAnimationsTable::EmplaceRetType
//SpriteAnimationsTable::Emplace(std::string_view seriesName, SpriteAnimationSeries&& series)
//{
//    auto result = map_.emplace(std::string{seriesName}, std::move(series));
//    if (result.second)
//    {
//        ClampSpriteRangeToSeriesSize(result.first->second.spriteRange, result.first->second);
//
//        if (map_.size() == 1)
//        {
//            current_ = result.first->first;
//            dirty_ = true;
//        }
//    }
//
//    return result;
//}
//
//bool SpriteAnimationsTable::Erase(std::string_view seriesName)
//{
//    auto it = map_.find(seriesName);
//    if (it == map_.end())
//    {
//        return false;
//    }
//
//    if (current_ == seriesName)
//    {
//        current_ = {};
//        dirty_ = true;
//    }
//
//    map_.erase(it);
//    return true;
//}
//
//// safe to dereference immediately if HasCurrentSeries() was checked first
//const SpriteAnimationSeries* SpriteAnimationsTable::GetCurrentSeries() const
//{
//    auto it = map_.find(current_);
//
//    return (it != map_.end()) ? &it->second : nullptr;
//}
//
//bool SpriteAnimationsTable::SetCurrentSeries(std::string_view seriesName, ResetOption resetOptions)
//{
//    auto it = map_.find(seriesName);
//    if (it == map_.end())
//    {
//        return false;
//    }
//
//    auto originalIt = map_.find(current_);
//    if (originalIt == map_.end()) // did not have a current set
//    {
//        current_ = it->first;
//        dirty_ = true;
//
//        return true;
//    }
//
//    // already had something set as current 
//    if (it == originalIt)
//    {
//        // no actual change, dont need to update
//        return true;
//    }
//
//    // setting a new series as current
//    if (resetOptions & ResetOption::Index)
//    {
//        originalIt->second.index = 0;
//    }
//    if (resetOptions & ResetOption::SpriteRange)
//    {
//        ResetCurrentSpriteRange();
//    }
//
//    current_ = it->first;
//    dirty_ = true;
//
//    return true;
//}
//
//bool SpriteAnimationsTable::SetCurrentSeries(std::string_view seriesName)
//{
//    return SetCurrentSeries(seriesName, ResetOption::All);
//}
//
//bool SpriteAnimationsTable::HasCurrentSeries() const
//{
//    return !current_.empty() && map_.contains(current_);
//}
//
//bool SpriteAnimationsTable::NextInSeries()
//{
//    auto it = map_.find(current_); 
//    if (it == map_.end())
//    {
//        return false;
//    }
//
//    auto& series = it->second;
//
//    assert(series.spriteRange.min <= series.spriteRange.max);
//
//    // if spriteRange was changed but index not bumped into it, do it now
//    if (!IndexInSpriteRange(series.index, series.spriteRange))
//    {
//        series.index = series.spriteRange.min;
//
//        // this action becomes our "next in series" operation, so return
//        return true;
//    }
//
//    size_t rangeDiff = series.spriteRange.max - series.spriteRange.min;
//
//    if (series.spritePlots.size() <= 1 || rangeDiff < 1)
//    {
//        return false; // stayed on the same sprite
//    }
//
//    AdvanceSeriesIndex(series);
//    dirty_ = true;
//    
//    return true;
//}
//
//bool SpriteAnimationsTable::SetCurrentIndex(size_t newIndex)
//{
//    auto it = map_.find(current_); 
//    if (it == map_.end())
//    {
//        return false;
//    }
//
//    auto& series = it->second;
//
//    if (series.spritePlots.empty() || !IndexInSpriteRange(newIndex, series.spriteRange))
//    {
//        return false;
//    }
//
//    if (series.index == newIndex)
//    {
//        return false;
//    }
//
//    series.index = newIndex;
//    dirty_ = true;
//
//    return true;
//}
//
//bool SpriteAnimationsTable::SetCurrentSpriteRange(Range<size_t> newRange, bool bumpIndexNow)
//{
//    assert(newRange.min <= newRange.max);
//
//    auto it = map_.find(current_);
//    if (it == map_.end())
//    {
//        return false;
//    }
//
//    auto& series = it->second;
//
//    ClampSpriteRangeToSeriesSize(newRange, series);
//
//    series.spriteRange = newRange;
//
//    if (bumpIndexNow && !IndexInSpriteRange(series.index, series.spriteRange))
//    {
//        series.index = series.spriteRange.min;
//
//        //series.index = (series.index < series.spriteRange.min) ? series.spriteRange.min :
//        //               (series.index > series.spriteRange.max) ? series.spriteRange.max :
//        //                                                         series.index;
//    }
//
//    return true;
//}
//
//bool SpriteAnimationsTable::SetCurrentSpriteRange(Range<size_t> newRange)
//{
//    return SetCurrentSpriteRange(newRange, true);
//}
//
//void SpriteAnimationsTable::ResetCurrentSpriteRange()
//{
//    auto it = map_.find(current_);
//    if (it == map_.end())
//    {
//        return;
//    }
//
//    auto& series = it->second;
//
//    series.spriteRange = { .min = 0, .max = series.spritePlots.size() };
//}
//
//bool SpriteAnimationsTable::Contains(std::string_view seriesName) const
//{
//    return map_.contains(seriesName);
//}
