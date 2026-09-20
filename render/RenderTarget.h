#pragma once
#include "../sdl/SDLite.h"
#include "../core/Result.h"

struct RenderTarget
{
	int width = 0;
	int height = 0;
	UniqueTexturePtr texture;

	bool operator!() const { return texture == nullptr; }
	operator bool() const { return texture != nullptr; }
	operator SDL_Texture* () const { return texture.get(); }

	static RenderTarget Create(SDL_Renderer* renderer, int w, int h)
	{
		return RenderTarget{
			.width = w,
			.height = h,
			.texture = MakeUniqueTexturePtr(
				renderer,
				SDL_PIXELFORMAT_RGBA8888,
				SDL_TEXTUREACCESS_TARGET,
				w,
				h)
		};
	}
};