#pragma once
#include <SDL.h>
#include <concepts>
#include "../core/TypeUtils.h"

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

    double delta = static_cast<double>(now - last) / 
                   static_cast<double>(SDL_GetPerformanceFrequency());

    last = now;

    return delta;
};

template <typename T>
concept SDLPointType = std::same_as<T, SDL_Point> || std::same_as<T, SDL_FPoint>;

template <typename T>
concept SDLRectType = std::same_as<T, SDL_Rect> || std::same_as<T, SDL_FRect>;

namespace SDLite
{
    static constexpr SDL_Color kColorRed    = { 255, 0,   0,   255 };
    static constexpr SDL_Color kColorGreen  = { 0,   255, 0,   255 };
    static constexpr SDL_Color kColorBlue   = { 0,   0,   255, 255 };
    static constexpr SDL_Color kColorYellow = { 255, 165, 0,   255 };
    static constexpr SDL_Color kColorWhite  = { 255, 255, 255, 255 };
    static constexpr SDL_Color kColorBlack  = { 0,   0,   0,   255 };
    static constexpr SDL_Color kColorPink   = { 238, 130, 238, 255 };
    static constexpr SDL_Color kColorPurple = { 106, 90,  205, 255 };
    static constexpr SDL_Color kColorOrange = { 255, 104, 25,  255 };
    static constexpr SDL_Color kColorBrown  = { 151, 75,  0,   255 };
}

// SDL POINT/RECT OVERLOADS

// Point on Point
template <SDLPointType P>
inline constexpr P operator-(P lhs, P rhs)
{
    return P{
        lhs.x - rhs.x,
        lhs.y - rhs.y
    };
}
template <SDLPointType P>
inline constexpr P operator+(P lhs, P rhs)
{
    return P{
        lhs.x + rhs.x,
        lhs.y + rhs.y
    };
}

// Scalar
template <SDLPointType P, ArithmeticType T>
inline constexpr P operator/(P p, T t)
{
    using ValueType = std::remove_cvref_t<decltype(P::x)>;

    return P{
        static_cast<ValueType>(static_cast<float>(p.x) / static_cast<float>(t)),
        static_cast<ValueType>(static_cast<float>(p.y) / static_cast<float>(t))
    };
}

template <SDLPointType P, ArithmeticType T>
inline constexpr P operator*(P p, T t)
{
    using ValueType = std::remove_cvref_t<decltype(P::x)>;

    return P{
        static_cast<ValueType>(static_cast<float>(p.x) * static_cast<float>(t)),
        static_cast<ValueType>(static_cast<float>(p.y) * static_cast<float>(t))
    };
}

// Mutating Point on Point
template <SDLPointType P>
inline constexpr P& operator+=(P& lhs, P rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;

    return lhs;
}

template <SDLPointType P>
inline constexpr P& operator-=(P& lhs, P rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;

    return lhs;
}

// Mutating Scalar
template <SDLPointType P, ArithmeticType T>
inline constexpr P& operator*=(P& p, T scalar)
{
    using ValueType = std::remove_cvref_t<decltype(P::x)>;

    p.x *= static_cast<ValueType>(scalar);
    p.y *= static_cast<ValueType>(scalar);

    return p;
}

// other
template <SDLPointType P>
inline constexpr P operator-(P p)
{
    return P{ -p.x, -p.y };
}