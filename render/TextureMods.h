#pragma once
#include <SDL.h>
#include "RGBA.h"

//template <typename T, typename U>
//inline constexpr T ClampValue(U val)
//{
//    return static_cast<T>(std::max(
//        std::min(val, std::numeric_limits<U>::max()),
//        std::numeric_limits<U>::min()
//    ));
//}
//
//template <typename T> requires std::is_arithmetic_v<T>
//struct RGB
//{
//    T r = static_cast<T>(255), g = static_cast<T>(255), b = static_cast<T>(255);
//
//    friend constexpr bool operator==(const RGB& lhs, const RGB& rhs)
//    {
//        return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
//    }
//    friend constexpr RGB operator+(const RGB& lhs, const RGB& rhs)
//    {
//        return RGB{
//            .r = lhs.r + rhs.r,
//            .g = lhs.g + rhs.g,
//            .b = lhs.b + rhs.b
//        };
//    }
//    friend constexpr RGB operator-(const RGB& lhs, const RGB& rhs)
//    {
//        return RGB{
//            .r = lhs.r - rhs.r,
//            .g = lhs.g - rhs.g,
//            .b = lhs.b - rhs.b
//        };
//    }
//
//    template <typename U> requires (std::convertible_to<T, U> &&
//                                    std::is_arithmetic_v<U>)
//    friend constexpr RGB operator*(const RGB& lhs, const U& rhs)
//    {
//        U newR = static_cast<U>(lhs.r) * rhs;
//        U newG = static_cast<U>(lhs.g) * rhs;
//        U newB = static_cast<U>(lhs.b) * rhs;
//
//        return RGB{
//            .r = ClampValue<T>(newR),
//            .g = ClampValue<T>(newG),
//            .b = ClampValue<T>(newB)
//        };
//    }
//};

struct TextureMods
{
    RGB color = {};
    int alpha = 255;
    SDL_BlendMode blend = SDL_BLENDMODE_BLEND;

    static constexpr SDL_Color ToSDLColor(const TextureMods& mods)
    {
        return SDL_Color{
            ClampToLimits<uint8_t>(mods.color.r),
            ClampToLimits<uint8_t>(mods.color.g),
            ClampToLimits<uint8_t>(mods.color.b),
            ClampToLimits<uint8_t>(mods.alpha)
        };
    }

    friend constexpr bool operator==(const TextureMods& lhs, const TextureMods& rhs)
    {
        return lhs.color == rhs.color && lhs.blend == rhs.blend;
    }
};

