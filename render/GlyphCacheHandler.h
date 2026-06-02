#pragma once
#include <SDL_rect.h>
#include <string_view>
#include <vector>
#include "../core/Anchor.h"

class FontAtlasTexture;
struct Transform;
struct TextRenderableComponent;
struct RenderProfile;
struct TextRenderableGlyphCache;
struct GlyphCacheData;

class GlyphCacheHandler
{
public:
	static void UpdateGlyphCache(const FontAtlasTexture& glyphAtlas,
						  TextRenderableComponent& textRenderable,
						  TextRenderableGlyphCache& glyphCache, 
								 const Transform& transform);

	static void RepopulateGlyphCacheGlyphs(std::string_view text,
										   std::vector<GlyphCacheData>& cache,
										   const FontAtlasTexture& glyphAtlas);

	static void ReprojectGlyphCacheGeometry(TextRenderableGlyphCache& cacheComponent,
											const TextRenderableComponent& textRenderable,
											const FontAtlasTexture& glyphAtlas);

	static void ScaleGlyphCache(std::vector<GlyphCacheData>& cache, Anchor scaleAnchor,
								SDL_FPoint tfScale);

	static void RotateGlyphCache(std::vector<GlyphCacheData>& cache, Anchor rotateAnchor,
								 float angleDegrees);

	static void RepositionGlyphCache(TextRenderableGlyphCache& cacheComponent,
									 SDL_FPoint newPos, SDL_FPoint newOffset);
};