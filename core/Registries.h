#pragma once
#include "../Core.h"

class GlyphAtlas;
class SpriteSeriesAtlas;

namespace registry {

using HandleRegistry = TypeList<
	GlyphAtlas,
	SpriteSeriesAtlas
>;

}