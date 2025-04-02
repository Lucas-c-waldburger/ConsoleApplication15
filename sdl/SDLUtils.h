#pragma once
#include <SDL.h>
#include <concepts>

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

static double GetDeltaTime()
{
    static uint64_t last = SDL_GetPerformanceCounter();

    uint64_t now = SDL_GetPerformanceCounter();

    double delta = static_cast<double>((now - last) * 1000.0 / 
                   static_cast<double>(SDL_GetPerformanceFrequency()));

    last = now;

    return delta;
};

template <typename T>
concept SDLPointType = std::same_as<T, SDL_Point> || std::same_as<T, SDL_FPoint>;

template <typename T>
concept SDLRectType = std::same_as<T, SDL_Rect> || std::same_as<T, SDL_FRect>;