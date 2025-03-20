#pragma once
#include <SDL.h>

static SDL_Color GetRenderDrawColor(SDL_Renderer* renderer)
{
	SDL_Color clr;
	SDL_GetRenderDrawColor(renderer, &clr.r, &clr.g, &clr.b, &clr.a);

	return clr;
}

static void SetRenderDrawColor(SDL_Renderer* renderer, SDL_Color clr)
{
	SDL_SetRenderDrawColor(renderer, clr.r, clr.g, clr.b, clr.a);
}