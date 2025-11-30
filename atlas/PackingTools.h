#pragma once
#include <SDL_rect.h>
#include "../deps/RectangleBinPack/MaxRectsBinPack.h"

constexpr inline void SwapWidthHeight(SDL_Rect& rect)
{
	int w = rect.w;
	rect.w = rect.h;
	rect.h = w;
}

constexpr inline bool WasFlipped(rbp::Rect target, SDL_Rect reference)
{
	return target.width == reference.h && target.height == reference.w;
}

constexpr inline bool WasRectPacked(rbp::Rect rect)
{
	return !(rect.width == 0 || rect.height == 0);
}

constexpr inline SDL_Rect RbpToSDLRect(rbp::Rect rect)
{
	return { rect.x, rect.y, rect.width, rect.height };
}

constexpr inline rbp::Rect SDLToRbpRect(SDL_Rect rect)
{
	return { rect.x, rect.y, rect.w, rect.h };
}

constexpr inline rbp::RectSize SDLToRbpRectSize(SDL_Rect rect)
{
	return { rect.w, rect.h };
}