#include "SpriteAnimationSeriesMap.h"

SpriteAnimationSeriesMap::EmplaceRetType  
SpriteAnimationSeriesMap::Emplace(std::string_view seriesName, const SpriteAnimationSeries& series)
{
    auto result = map_.emplace(std::string{seriesName}, series);
    if (result.second)
    {
        current_ = result.first->first;
    }

    return result;
}

SpriteAnimationSeriesMap::EmplaceRetType 
SpriteAnimationSeriesMap::Emplace(std::string_view seriesName, SpriteAnimationSeries&& series)
{
    auto result = map_.emplace(std::string{seriesName}, std::move(series));
    if (result.second)
    {
        current_ = result.first->first;
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
    }

    map_.erase(it);
    return true;
}

// safe to dereference immediately if HasCurrent() was checked first
SpriteAnimationSeries* SpriteAnimationSeriesMap::GetCurrent()
{
    auto it = map_.find(current_);

    return (it != map_.end()) ? &it->second : nullptr;
}

const SpriteAnimationSeries* SpriteAnimationSeriesMap::GetCurrent() const
{
    auto it = map_.find(current_);

    return (it != map_.end()) ? &it->second : nullptr;
}

bool SpriteAnimationSeriesMap::SetCurrent(std::string_view seriesName, bool resetOriginal)
{
    auto it = map_.find(seriesName);
    if (it != map_.end())
    {
        auto originalIt = map_.find(current_);
        if (originalIt != map_.end())
        {
            if (resetOriginal)
            {
                originalIt->second.index = 0;
            }
        }

        current_ = it->first;
        return true;
    }

    return false;
}

bool SpriteAnimationSeriesMap::HasCurrent() const
{
    return !current_.empty() && map_.contains(current_);
}

SpriteAnimationSeries* SpriteAnimationSeriesMap::operator[](std::string_view seriesName)
{
    auto it = map_.find(seriesName);

    return (it != map_.end()) ? &it->second : nullptr;
}

const SpriteAnimationSeries* SpriteAnimationSeriesMap::operator[](std::string_view seriesName) const
{
    auto it = map_.find(seriesName);

    return (it != map_.end()) ? &it->second : nullptr;
}

bool SpriteAnimationSeriesMap::Contains(std::string_view seriesName) const
{
    return map_.contains(seriesName);
}
