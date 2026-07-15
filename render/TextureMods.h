#pragma once
#include <SDL.h>
#include "RGBA.h"

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

    friend constexpr bool operator==(const TextureMods& lhs, const TextureMods& rhs) noexcept
    {
        return lhs.color == rhs.color && lhs.blend == rhs.blend;
    }
};

