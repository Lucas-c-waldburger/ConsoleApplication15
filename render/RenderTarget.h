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

struct RenderTargetState
{
	Dimensions<int> targetDimensions = { 0, 0 };
	SDL_FRect displayArea = { 0.0f, 0.0f, 0.0f, 0.0f };

	constexpr bool Valid() const
	{
		return targetDimensions.w > 0 && targetDimensions.h > 0 &&
			   displayArea.w > 0 && displayArea.w <= targetDimensions.w &&
			   displayArea.h > 0 && displayArea.h <= targetDimensions.h;
	}

	constexpr float GetAspectRatio() const
	{
		if (targetDimensions.h == 0)
		{
			return 0;
		}

		return static_cast<float>(targetDimensions.w) / static_cast<float>(targetDimensions.h);
	}
};