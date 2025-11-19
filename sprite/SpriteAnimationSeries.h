#pragma once
#include <bitset>
#include <algorithm>
#include "../core/Literals.h"
#include "../sdl/SDLUtils.h"
#include "../events/data/SpriteAnimationEvents.h"
#include "../atlas/NewAtlas.h"


// MAKE THIS A SEPARATE COMPONENT
struct SeriesMetrics
{
	std::optional<ThresholdTracker<float>> time;
	std::optional<ThresholdTracker<float, SDL_FPoint>> distance;
};

struct SpriteAnimationSeries
{
	Handle<TextureAtlas> sourceAtlas;
	std::vector<AtlasPlot> spritePlots;
	size_t index = 0;
	Range<size_t> spriteRange = { 0, 0 };
	std::bitset<EventDataTypeList::size> eventProductionFlags;
	SeriesMetrics seriesMetrics;
};

