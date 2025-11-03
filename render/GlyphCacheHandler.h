#pragma once
#include <SDL_rect.h>
#include <string_view>
#include <vector>

class NewGlyphAtlas;
struct Transform;
struct TextRenderableComponent;
struct RenderProfile;
struct TextRenderableGlyphCache;
struct GlyphCacheData;

class GlyphCacheHandler
{
public:
	static void UpdateGlyphCache(const NewGlyphAtlas& glyphAtlas,
								 TextRenderableComponent& textRenderable,
								 TextRenderableGlyphCache& glyphCache, 
								 const Transform& transform);

	static void RepopulateGlyphCacheGlyphs(std::string_view text,
										   std::vector<GlyphCacheData>& cache,
										   const NewGlyphAtlas& glyphAtlas);

	static void ReprojectGlyphCacheGeometry(std::vector<GlyphCacheData>& cache,
											const TextRenderableComponent& textRenderable,
											const Transform& transform,
											SDL_Rect projectedRect,
											const NewGlyphAtlas& glyphAtlas);

	static void AdjustGlyphCacheRotation(std::vector<GlyphCacheData>& cache,
										 SDL_Rect projectedRenderRect, float angleDegrees);

	static void AdjustGlyphCachePosition(TextRenderableGlyphCache& cacheComponent,
										 SDL_FPoint newPos, SDL_FPoint newOffset);
};