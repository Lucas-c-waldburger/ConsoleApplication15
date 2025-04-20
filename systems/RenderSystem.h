#pragma once
#include "../sdl/SDLUtils.h"
#include "../core/Result.h"
#include "../atlas/AtlasManager.h"
#include <span>
#include <numeric>
#include <typeindex>

//namespace impl {
//class atlasStore;
//}

class GlyphAtlas;


class RenderSystem 
{
public:
	void Update(SDL_Renderer* renderer, const impl::AtlasStore& atlasStore);
	
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