#pragma once
#include <algorithm>
#include "../atlas/SpriteSeriesAtlas.h"
#include "../core/Literals.h"
#include "../sdl/SDLUtils.h"
#include "../events/data/SpriteAnimationEvents.h"
#include "../core/EventDataBitset.h"

struct SpriteAnimationSeries
{
	Handle<SpriteSeriesAtlas> sourceAtlas;
	std::vector<AtlasPlot> spritePlots;
	size_t index = 0;
	Range<size_t> spriteRange = { 0, 0 };
	std::bitset<EventDataTypeList::size> eventProductionFlags;
};

