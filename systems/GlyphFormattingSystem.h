#pragma once
#include "System.h"
#include <SDL_rect.h>
#include <string_view>
#include <vector>

class Entity;
class NewGlyphAtlas;

struct Transform;
struct NewTextRenderable;
struct GlyphCache;
struct GlyphCacheData;

class GlyphFormattingSystem : public System
{
public:
	void Update(const NewGlyphAtlas& glyphAtlas);

	static void RepopulateGlyphCacheGlyphs(std::string_view text, 
										   std::vector<GlyphCacheData>& cache,
										   const NewGlyphAtlas& glyphAtlas);

	static void ReprojectGlyphCacheGeometry(std::vector<GlyphCacheData>& cache,
										    const NewTextRenderable& textRenderable,
										    const Transform& transform,
										    SDL_Rect projectedRect,
										    const NewGlyphAtlas& glyphAtlas);

	static void AdjustGlyphCacheRotation(std::vector<GlyphCacheData>& cache,
									     SDL_Rect projectedRenderRect, float angleDegrees);

	static void AdjustGlyphCachePosition(GlyphCache& cacheComponent, SDL_FPoint newPos,
								         SDL_FPoint newOffset);
};