#pragma once
#include "../sdl/SDLUtils.h"
#include "../core/Result.h"
#include "../atlas/AtlasManager.h"
#include "System.h"
#include <span>
#include <numeric>
#include <typeindex>

class GlyphAtlas;
class Camera;

class RenderSystem : public System
{
public:
	// TODO: Attach stores to render system via Scene
	void Update(SDL_Renderer* renderer, const Camera& camera, const impl::AtlasStore& atlasStore);
	
private:	
	struct RenderGlyphsArgs
	{
		SDL_Renderer* renderer = nullptr;
		const GlyphAtlas* glyphAtlas;
		std::string_view text;
		SDL_FPoint scale = { 0.0f, 0.0f };
		int numNewlines = 0;
		int startY = 0;
		int startX = 0;
	};

	static float GetScaleToFitFactor(const RenderGlyphsArgs& args, int totalHeight,
									 int boundingWidth, int boundingHeight);
};