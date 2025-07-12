#include <algorithm>
#include "SpriteAnimationDriver.h"
#include "../SpriteAnimationsComponent.h"
#include "../NeedsUpdateComponent.h"
#include "../../events/EventBus.h"
#include "../../events/data/SpriteAnimationEvents.h"

namespace {

void ClampSpriteRangeToSeriesSize(Range<size_t>& spriteRange, const SpriteAnimationSeries& series)
{
	spriteRange.min = std::clamp(spriteRange.min, 0_uz, series.spritePlots.size() - 1);
	spriteRange.max = std::clamp(spriteRange.max, 0_uz, series.spritePlots.size() - 1);
}

void AdvanceSeriesIndex(SpriteAnimationSeries& series)
{
	series.index = (series.index + 1 > series.spriteRange.max)
		? series.spriteRange.min
		: series.index + 1;
}

constexpr bool IndexInSpriteRange(size_t index, const Range<size_t>& spriteRange)
{
	return index >= spriteRange.min && index <= spriteRange.max;
}

bool ShouldProduceSeriesChangeEvent(const SpriteAnimationSeries& oldSeries,
								    const SpriteAnimationSeries& newSeries)
{
	return oldSeries.eventProductionFlags.test(events::SpriteSeriesChange::eventType) ||
		   newSeries.eventProductionFlags.test(events::SpriteSeriesChange::eventType);
}

} // unnamed

bool SpriteAnimationDriver::AddSeries(std::string_view seriesName, const SpriteSeriesAtlas& spriteAtlas)
{
	HashName seriesNameHashed{ seriesName };

	auto& animations = GetComponent<SpriteAnimations>();
	if (animations.table.contains(seriesNameHashed))
	{
		return false;
	}

	auto spritePlots = spriteAtlas.GetSpritePlots(seriesName);
	if (spritePlots.empty())
	{
		return false;
	}

	auto& newSeries = animations.table[seriesNameHashed];

	newSeries.sourceAtlas = spriteAtlas.GetHandle();
	newSeries.spritePlots = std::move(spritePlots);
	newSeries.spriteRange = { .min = 0, .max = newSeries.spritePlots.size() - 1 };

	if (animations.table.size() == 1)
	{
		animations.current = seriesNameHashed;

		MarkNeedsUpdate();
	}
	
	return true;
}

bool SpriteAnimationDriver::RemoveSeries(std::string_view seriesName)
{
	auto& animations = GetComponent<SpriteAnimations>();

	auto it = animations.table.find(HashName{ seriesName });
	if (it == animations.table.end())
	{
		return false;
	}

	if (it->first == animations.current)
	{
		if (!animations.table.empty())
		{
			animations.current = animations.table.begin()->first;
		}
		else
		{
			animations.current = kInvalidHashName;
		}

		MarkNeedsUpdate();
	}

	animations.table.erase(it);

	return true;
}

bool SpriteAnimationDriver::HasCurrentSeries() const
{
	const auto& animations = GetComponent<SpriteAnimations>();

	return animations.table.contains(animations.current);
}

bool SpriteAnimationDriver::SetCurrentSeries(std::string_view seriesName, ResetOption resetOptions)
{
	auto& animations = GetComponent<SpriteAnimations>();

	HashName seriesNameHash{ seriesName };
	bool alreadySet = animations.current == seriesNameHash;

	auto originalIt = animations.table.find(animations.current);
	if (originalIt == animations.table.end())
	{
		if (alreadySet) // sanity check if current doesn't map to a real series
		{
			animations.current = kInvalidHashName; 

			MarkNeedsUpdate();
		}

		return false;
	}

	if (alreadySet)
	{
		return false;
	}

	auto& [oldName, oldSeries] = *originalIt;
	if (resetOptions & ResetOption::Index)
	{
		oldSeries.index = 0;
	}
	if (resetOptions & ResetOption::SpriteRange)
	{
		oldSeries.spriteRange = { .min = 0, .max = oldSeries.spritePlots.size() };
	}

	animations.current = seriesNameHash;

	auto& newSeries = animations.table[animations.current];

	if (ShouldProduceSeriesChangeEvent(oldSeries, newSeries))
	{
		EventBus::PushEvent(events::SpriteSeriesChange{
			.entity = GetEntity().GetID(),
			.seriesName = { .last = oldName, .now = animations.current }
		});
	}

	MarkNeedsUpdate();

	return true;
}

bool SpriteAnimationDriver::SetCurrentSeries(std::string_view seriesName)
{
	return SetCurrentSeries(seriesName, ResetOption::All);
}

bool SpriteAnimationDriver::SetCurrentSeriesIndex(size_t newIndex)
{
	auto& animations = GetComponent<SpriteAnimations>();

	auto it = animations.table.find(animations.current);
	if (it == animations.table.end())
	{
		return false;
	}

	auto& series = it->second;
	assert(SeriesValid(series));

	if (!IndexInSpriteRange(newIndex, series.spriteRange))
	{
		return false;
	}
	if (series.index == newIndex)
	{
		return false;
	}

	size_t originalIndex = series.index;
	series.index = newIndex;

	if (series.eventProductionFlags.test(events::SpriteIndexChange::eventType))
	{
		EventBus::PushEvent(events::SpriteIndexChange{
			.entity = GetEntity().GetID(),
			.seriesName = animations.current,
			.index = { .last = originalIndex, .now = series.index }
		});
	}

	MarkNeedsUpdate();

	return true;
}

bool SpriteAnimationDriver::SetCurrentSeriesRange(const Range<size_t>& newRange)
{
	if (newRange.min > newRange.max)
	{
		return false;
	}

	auto& animations = GetComponent<SpriteAnimations>();

	auto it = animations.table.find(animations.current);
	if (it == animations.table.end())
	{
		return false;
	}

	auto& series = it->second;
	assert(SeriesValid(series));

	if (newRange.max >= series.spritePlots.size())
	{
		return false;
	}

	series.spriteRange = newRange;
	if (!IndexInSpriteRange(series.index, series.spriteRange))
	{
		return SetCurrentSeriesIndex(series.spriteRange.min);
	}

	return true;
}

bool SpriteAnimationDriver::Step()
{
	auto& animations = GetComponent<SpriteAnimations>();

	auto it = animations.table.find(animations.current);
	if (it == animations.table.end())
	{
		return false;
	}

	auto& series = it->second;
	assert(SeriesValid(series));

	if (series.spriteRange.max - series.spriteRange.min < 1)
	{
		return false; // stayed on the same sprite
	}

	size_t originalIndex = series.index;

	AdvanceSeriesIndex(series);

	if (series.eventProductionFlags.test(events::SpriteIndexChange::eventType))
	{
		EventBus::PushEvent(events::SpriteIndexChange{
			.entity = GetEntity().GetID(),
			.seriesName = animations.current,
			.index = { .last = originalIndex, .now = series.index }
		});
	}

	MarkNeedsUpdate();

	return true;
}

bool SpriteAnimationDriver::SeriesValid(const SpriteAnimationSeries& series)
{
	bool validHandle = series.sourceAtlas.IsValid();
	bool validPlots = !series.spritePlots.empty();
	bool validIndex = IndexInSpriteRange(series.index, series.spriteRange);
	bool validSpriteRange = series.spriteRange.min <= series.spriteRange.max &&
		series.spriteRange.max < series.spritePlots.size();

	return validHandle && validPlots && validIndex && validSpriteRange;
}

void SpriteAnimationDriver::MarkNeedsUpdate()
{
	auto& update = AddComponent<NeedsUpdate>();

	update.components |= SpriteAnimations::componentBit;
}