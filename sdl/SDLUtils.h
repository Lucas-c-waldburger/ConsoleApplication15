#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <concepts>
#include "../core/TypeUtils.h"
#include "../core/CommonFunctions.h"

// unique sdl ptr wrappers
// surface
using UniqueSurfacePtr = std::unique_ptr<SDL_Surface, 
    decltype([](SDL_Surface* s) { if (s) { SDL_FreeSurface(s); } })>;

inline UniqueSurfacePtr MakeUniqueSurfacePtr(std::string_view filepath)
{
    return UniqueSurfacePtr{ IMG_Load(filepath.data()) };
}
inline UniqueSurfacePtr MakeUniqueSurfacePtrBMP(const std::string& bmpFileName)
{
    return UniqueSurfacePtr{ SDL_LoadBMP(bmpFileName.c_str()) };
}

// texture
using UniqueTexturePtr = std::unique_ptr < SDL_Texture,
    decltype([](SDL_Texture* t) { SDL_DestroyTexture(t); })> ;

inline UniqueTexturePtr
MakeUniqueTexturePtr(SDL_Renderer* renderer, SDL_PixelFormatEnum fmt,
                     SDL_TextureAccess access, int w, int h)
{
    return UniqueTexturePtr{ SDL_CreateTexture(renderer, fmt, access, w, h) };
}
inline UniqueTexturePtr MakeUniqueTexturePtrFromSurface(SDL_Renderer* renderer, SDL_Surface* surface)
{
    return UniqueTexturePtr{ SDL_CreateTextureFromSurface(renderer, surface) };
}

// cursor
using UniqueCursorPtr = std::unique_ptr<SDL_Cursor, 
    decltype([](SDL_Cursor* crsr) { if (crsr) { SDL_FreeCursor(crsr); } })>;

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

// SDL COLOR OVERLOADS
inline constexpr bool operator==(const SDL_Color& lhs, const SDL_Color& rhs) noexcept
{
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}
//

// SDL POINT/RECT OVERLOADS
// POINT //
// equality
inline constexpr bool operator==(const SDL_Point& lhs, const SDL_Point& rhs) noexcept
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline constexpr bool operator==(const SDL_FPoint& lhs, const SDL_FPoint& rhs) noexcept
{
    return EqualsWithTolerance(lhs.x, rhs.x) && EqualsWithTolerance(lhs.y, rhs.y);
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
template <SDLPointType P>
inline constexpr P operator*(P lhs, P rhs)
{
    return P{
        lhs.x * rhs.x,
        lhs.y * rhs.y
    };
}
template <SDLPointType P>
inline constexpr P operator/(P lhs, P rhs)
{
    return P{
        lhs.x / rhs.x,
        lhs.y / rhs.y
    };
}

// Scalar
template <SDLPointType P, ArithmeticType T>
inline constexpr P operator+(P p, T t)
{
    using ValueType = std::remove_cvref_t<decltype(P::x)>;

    return P{
        static_cast<ValueType>(static_cast<float>(p.x) + static_cast<float>(t)),
        static_cast<ValueType>(static_cast<float>(p.y) + static_cast<float>(t))
    };
}

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

template <SDLPointType P>
inline constexpr P& operator*=(P& lhs, P rhs)
{
    lhs.x *= rhs.x;
    lhs.y *= rhs.y;

    return lhs;
}

template <SDLPointType P>
inline constexpr P& operator/=(P& lhs, P rhs)
{
    lhs.x /= rhs.x;
    lhs.y /= rhs.y;

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

namespace std {

template <SDLPointType P>
constexpr P abs(P p)
{
    return P{ std::abs(p.x), std::abs(p.y) };
}

} // std

// RECT //
// comparison
template <SDLRectType T>
inline constexpr bool operator==(const T& lhs, const T& rhs) noexcept
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.w == rhs.w && lhs.h == rhs.h;
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