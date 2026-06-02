#pragma once
#include "../components/TransformComponent.h"
#include "../components/RenderableComponent.h"
#include "../atlas/NewGlyphAtlas.h"

namespace util {

bool operator==(std::string_view text, const std::vector<GlyphCacheData>& cacheData);

enum ChangeLog : uint8_t
{
	NoChange = 0,
	TextChanged = 1 << 0,
	FormatChanged = 1 << 1,
	RotationChanged = 1 << 2,
	PositionChanged = 1 << 3,
	ScaleChanged = 1 << 4,
	AtlasChanged = 1 << 5,
	NeedsReprojection = FormatChanged | TextChanged | AtlasChanged,
	NeedsReposition = PositionChanged | NeedsReprojection | ScaleChanged | RotationChanged
};

uint8_t MakeChangeLog(const TextRenderableComponent& textRenderable,
					  const Transform& transform,
					  const TextRenderableGlyphCache& glyphCache);

struct FormatArgs
{
	Dimensions<int> bounds = { 0, 0 };
	SDL_FPoint layoutScale = { 1.0f, 1.0f };
	SDL_FPoint start = { 0, 0 };
	int numNewlines = 0;
	int fontHeight = 0;
	float totalHeight = 0.0f;
};

FormatArgs MakeFormatArgs(const TextRenderableComponent& textRenderable,
						  const TextRenderableGlyphCache& cacheComponent, int fontHeight);

float GetFormatArgsStartX(const TextRenderableComponent& textRenderable, const FormatArgs& formatArgs,
						  const std::vector<GlyphCacheData>& cache);

float CalculateGlyphRowWidth(const std::vector<GlyphCacheData>& cache,
						     size_t currentPos, size_t newlinePos, float scaleX);

float CalculateLongestRowWidth(std::string_view text, const FormatArgs& format,
							   const std::vector<GlyphCacheData>& cache);

float GetScaleToFitFactor(std::string_view text, const std::vector<GlyphCacheData>& cache,
						  const FormatArgs& formatArgs);

SDL_FRect ComputeGlyphDataBoundingBox(const std::vector<GlyphCacheData>& cache);

} // util