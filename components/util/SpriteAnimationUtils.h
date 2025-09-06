#pragma once
#include <cassert>
#include "../SpriteAnimationsComponent.h"


inline void AdvanceSpriteSeries(SpriteAnimationSeries& series)
{
	assert(series.spriteRange.max <= series.spritePlots.size());

	series.index = (series.index + 1) > series.spriteRange.max
		? series.spriteRange.min
		: series.index + 1;
}

inline bool SetCurrentSpriteSeries(SpriteAnimations& animations, std::string_view newSeriesName,
								   bool resetPrevSeries = true)
{
	if (newSeriesName == animations.current)
	{
		return false;
	}

	if (resetPrevSeries)
	{
		auto oldIt = animations.table.find(animations.current);
		if (oldIt != animations.table.end())
		{
			oldIt->second.index = oldIt->second.spriteRange.min;
		}
	}

	auto newIt = animations.table.find(newSeriesName);
	if (newIt == animations.table.end())
	{
		animations.current.clear();
		return true;
	}

	animations.current = std::string{ newSeriesName };

	return true;
}
