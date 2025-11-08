#pragma once
#include <SDL_rect.h>
#include <string_view>
#include <vector>
#include "../core/Anchor.h"

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

	static void ReprojectGlyphCacheGeometry(TextRenderableGlyphCache& cacheComponent,
											const TextRenderableComponent& textRenderable,
											const NewGlyphAtlas& glyphAtlas);

	static void ScaleGlyphCache(std::vector<GlyphCacheData>& cache, Anchor scaleAnchor,
								SDL_FPoint tfScale);

	static void RotateGlyphCache(std::vector<GlyphCacheData>& cache, Anchor rotateAnchor,
								 float angleDegrees);

	static void RepositionGlyphCache(TextRenderableGlyphCache& cacheComponent,
									 SDL_FPoint newPos, SDL_FPoint newOffset);
};