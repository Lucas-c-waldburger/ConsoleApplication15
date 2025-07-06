#pragma once
#include <SDL_rect.h>
#include <type_traits>

enum class Direction : uint8_t
{
    None = 0,
    N = 1 << 0,
    E = 1 << 1,
    S = 1 << 2,
    W = 1 << 3,
    NE = (N | E),
    NW = (N | W),
    SE = (S | E),
    SW = (S | W)
};

inline constexpr bool operator&(Direction lhs, Direction rhs)
{
    using UL = std::underlying_type_t<Direction>;

    return static_cast<UL>(lhs) & static_cast<UL>(rhs);
}
inline constexpr Direction operator|(Direction lhs, Direction rhs)
{
    using UL = std::underlying_type_t<Direction>;

    return static_cast<Direction>(static_cast<UL>(lhs) | static_cast<UL>(rhs));
}

inline constexpr Direction GetDirectionFromPoint(SDL_Point p)
{
    using enum Direction;

    Direction x = (p.x < 0) ? W : (p.x > 0) ? E : None;
    Direction y = (p.y < 0) ? N : (p.y > 0) ? S : None;

    return (x | y);
}

inline constexpr bool ShouldFlipHorizontally(Direction native, Direction input)
{
    using enum Direction;

    return ((native & E) && (input & W)) || ((native & W) && (input & E));
}
