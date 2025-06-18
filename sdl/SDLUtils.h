#pragma once
#include <SDL.h>
#include <concepts>
#include "../core/TypeUtils.h"

// unique sdl ptr wrappers
// surface
using SurfaceDtor = decltype([](SDL_Surface* surf) { if (surf) { SDL_FreeSurface(surf); } });
using UniqueSurfacePtr = std::unique_ptr<SDL_Surface, SurfaceDtor>;

inline UniqueSurfacePtr MakeUniqueSurfacePtrBMP(const std::string& bmpFileName)
{
    return UniqueSurfacePtr{ SDL_LoadBMP(bmpFileName.c_str()) };
}

// cursor
using CursorDtor = decltype([](SDL_Cursor* crsr) { if (crsr) { SDL_FreeCursor(crsr); } });
using UniqueCursorPtr = std::unique_ptr<SDL_Cursor, CursorDtor>;

inline UniqueCursorPtr MakeUniqueCursor(SDL_SystemCursor systemCursor)
{
    return UniqueCursorPtr{ SDL_CreateSystemCursor(systemCursor) };
}
inline UniqueCursorPtr MakeUniqueCursor(UniqueSurfacePtr& surface, SDL_Point clickOffset)
{
    return UniqueCursorPtr{ SDL_CreateColorCursor(surface.get(), clickOffset.x, clickOffset.y) };
}

// enum additions/extentions
enum SDL_MouseButton : uint32_t
{
    SDL_MOUSE_BUTTON_INVALID = 0,
    SDL_MOUSE_BUTTON_LEFT = SDL_BUTTON_LEFT,
    SDL_MOUSE_BUTTON_MIDDLE = SDL_BUTTON_MIDDLE,
    SDL_MOUSE_BUTTON_RIGHT = SDL_BUTTON_RIGHT,
    SDL_MOUSE_BUTTON_X1 = SDL_BUTTON_X1,
    SDL_MOUSE_BUTTON_X2 = SDL_BUTTON_X2,
    SDL_MOUSE_BUTTON_MAX = 6
};

// utility functions
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

// equality
template <SDLPointType P>
inline constexpr bool operator==(const P& lhs, const P& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

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

// RECT/POINT MATH //
template <SDLRectType T, SDLRectType U>
inline constexpr bool RectsIntersect(const T& a, const U& b) noexcept
{
    return (static_cast<float>(a.x) < static_cast<float>(b.x) + static_cast<float>(b.w) &&
            static_cast<float>(a.x) + static_cast<float>(a.w) > static_cast<float>(b.x) &&
            static_cast<float>(a.y) < static_cast<float>(b.y) + static_cast<float>(b.h) &&
            static_cast<float>(a.y) + static_cast<float>(a.h) > static_cast<float>(b.y));
}

template <SDLRectType R, SDLPointType P>
inline constexpr bool PointInsideRect(const R& rect, const P& point) noexcept
{
    return static_cast<float>(point.x) >= static_cast<float>(rect.x) &&
           static_cast<float>(point.x) <= static_cast<float>(rect.x + rect.w) &&
           static_cast<float>(point.y) >= static_cast<float>(rect.y) &&
           static_cast<float>(point.y) <= static_cast<float>(rect.y + rect.h);
}