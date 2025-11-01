#pragma once
#include "System.h"
#include <SDL_rect.h>
#include <string_view>
#include <vector>

class Entity;
class NewTextureRepository;
class NewGlyphAtlas;

struct Transform;
struct NewTextRenderable;
struct NewSpriteRenderable;
struct Sprite;
struct RenderProfile;
struct TextRenderableGlyphCache;
struct GlyphCacheData;
struct RenderCallArgs;

class GlyphCacheHandler
{
public:
	//void Update(const NewTextureRepository& textureRepo);
	static void UpdateGlyphCache(const NewGlyphAtlas& glyphAtlas,
								 NewRenderable& renderable, NewTextRenderable& textRenderable,
								 TextRenderableGlyphCache& glyphCache, const Transform& transform);
	//static bool UpdateEntityGlyphCache(Entity& entity, const NewGlyphAtlas& glyphAtlas);

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

	static void AdjustGlyphCachePosition(TextRenderableGlyphCache& cacheComponent, SDL_FPoint newPos,
								         SDL_FPoint newOffset);
};

class RenderablePreProcessor : public System
{
public:
	void Update(const NewTextureRepository& textureRepo);

private:
	static void UpdateSprite(NewRenderable& renderable, const Sprite& sprite,
						     const NewTextureRepository& textureRepo);
	static void UpdateGlyphs(NewRenderable& renderable, NewTextRenderable& textRenderable,
							 const NewTextureRepository& textureRepo);
};