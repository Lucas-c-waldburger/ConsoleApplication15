#include "SpriteAnimationSeriesMap.h"

SpriteAnimationSeriesMap::EmplaceRetType  
SpriteAnimationSeriesMap::Emplace(std::string_view seriesName, const SpriteAnimationSeries& series)
{
    auto result = map_.emplace(std::string{seriesName}, series);
    if (result.second && map_.size() == 1)
    {
        // set current if first animation set
        current_ = result.first->first;
        dirty_ = true;
    }

    return result;
}

SpriteAnimationSeriesMap::EmplaceRetType 
SpriteAnimationSeriesMap::Emplace(std::string_view seriesName, SpriteAnimationSeries&& series)
{
    auto result = map_.emplace(std::string{seriesName}, std::move(series));
    if (result.second && map_.size() == 1)
    {
        current_ = result.first->first;
        dirty_ = true;
    }

    return result;
}

bool SpriteAnimationSeriesMap::Erase(std::string_view seriesName)
{
    auto it = map_.find(seriesName);
    if (it == map_.end())
    {
        return false;
    }

    if (current_ == seriesName)
    {
        current_ = {};
        dirty_ = true;
    }

    map_.erase(it);
    return true;
}

// safe to dereference immediately if HasCurrent() was checked first
const SpriteAnimationSeries* SpriteAnimationSeriesMap::GetCurrent() const
{
    auto it = map_.find(current_);

    return (it != map_.end()) ? &it->second : nullptr;
}

bool SpriteAnimationSeriesMap::SetCurrent(std::string_view seriesName, bool resetOriginal)
{
    auto it = map_.find(seriesName);
    if (it == map_.end())
    {
        return false;
    }

    auto originalIt = map_.find(current_);
    if (originalIt == map_.end()) // did not have a current set
    {
        current_ = it->first;
        dirty_ = true;

        return true;
    }

    // already had something set as current 
    if (it == originalIt)
    {
        // no actual change, dont need to update
        return true;
    }

    // setting a new series as current
    if (resetOriginal)
    {
        originalIt->second.index = 0;
    }

    current_ = it->first;
    dirty_ = true;

    return true;
}

bool SpriteAnimationSeriesMap::SetCurrent(std::string_view seriesName)
{
    return SetCurrent(seriesName, true);
}

bool SpriteAnimationSeriesMap::HasCurrent() const
{
    return !current_.empty() && map_.contains(current_);
}

bool SpriteAnimationSeriesMap::NextInSeries()
{
    auto it = map_.find(current_); 
    if (it == map_.end())
    {
        return false;
    }

    auto& series = it->second;
    if (series.spritePlots.size() <= 1)
    {
        return false;
    }

    series.index = (series.index + 1) % series.spritePlots.size();
    dirty_ = true;
    
    return true;
}

bool SpriteAnimationSeriesMap::SetCurrentIndex(size_t newIndex)
{
    auto it = map_.find(current_); 
    if (it == map_.end())
    {
        return false;
    }

    auto& series = it->second;

    if (series.spritePlots.empty() || newIndex >= series.spritePlots.size())
    {
        return false;
    }

    if (series.index == newIndex)
    {
        return false;
    }

    series.index = newIndex;
    dirty_ = true;

    return true;
}

bool SpriteAnimationSeriesMap::Contains(std::string_view seriesName) const
{
    return map_.contains(seriesName);
}
