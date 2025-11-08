#pragma once
#include "../sdl/SDLUtils.h"

enum class Anchor : uint8_t
{
    Left = 1 << 0,
    Right = 1 << 1,
    Top = 1 << 2,
    Bottom = 1 << 3,
    CenterX = 1 << 4,   // horizontal center
    CenterY = 1 << 5,   // vertical center

    TopLeft = Top | Left,
    TopRight = Top | Right,
    BottomLeft = Bottom | Left,
    BottomRight = Bottom | Right,
    Center = CenterX | CenterY,
};

constexpr inline Anchor operator|(Anchor lhs, Anchor rhs)
{
    using UL = std::underlying_type_t<Anchor>;
    return static_cast<Anchor>(static_cast<UL>(lhs) | static_cast<UL>(rhs));
}

constexpr inline bool operator&(Anchor lhs, Anchor rhs)
{
    using UL = std::underlying_type_t<Anchor>;
    return static_cast<bool>(static_cast<UL>(lhs) & static_cast<UL>(rhs));
}

template <SDLRectType R>
constexpr inline SDL_FPoint GetRectAnchorPoint(R rect, Anchor anchor)
{
    SDL_FPoint p{ static_cast<float>(rect.x), static_cast<float>(rect.y) };

    // Horizontal
    if (anchor & Anchor::Right)
    {
        p.x += static_cast<float>(rect.w);
    }
    else if (anchor & Anchor::CenterX)
    {
        p.x += static_cast<float>(rect.w) * 0.5f;
    }

    // Vertical
    if (anchor & Anchor::Bottom)
    {
        p.y += static_cast<float>(rect.h);
    }
    else if (anchor & Anchor::CenterY)
    {
        p.y += static_cast<float>(rect.h) * 0.5f;
    }

    return p;
}