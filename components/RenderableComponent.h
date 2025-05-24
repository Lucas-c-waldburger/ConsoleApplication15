#pragma once
#include "BaseComponent.h"
#include "../core/Handle.h"
#include "../physics/B2Shape.h"
#include <variant>
#include <SDL.h>

class GlyphAtlas;
class SpriteSeriesAtlas;

// TODO: make draw order layers (overlay, etc.)

struct TextureMods
{
    uint8_t alpha = 255;
    struct RGB {
        uint8_t r = 255;
        uint8_t g = 255;
        uint8_t b = 255;

        friend constexpr bool operator==(const RGB& lhs, const RGB& rhs) = default;
    } color;
    SDL_BlendMode blend = SDL_BLENDMODE_BLEND;

    friend constexpr bool operator==(const TextureMods& lhs, const TextureMods& rhs) = default; 
};



struct Renderable : BaseComponent<Renderable, 7>
{
    struct Text
    {
        enum Alignment
        {
            Left,
            Center,
            Right
        };

        Handle<GlyphAtlas> sourceAtlas;
        std::string text;
        Dimensions<int> desiredDimensions = { 0, 0 }; // TODO: dont store this on here
        Alignment align;
        bool scaleToFit = false;
    };

    struct Sprite
    {
        Handle<SpriteSeriesAtlas> sourceAtlas;
        std::string seriesName;
        int currentIndex = -1;
    };

    struct Geometry
    {
        SDL_Color color = { 0, 0, 0, 255 };

        /*struct {
            bool draw = false;
            SDL_Color color = { 0, 0, 0, 255 };
        } lines;*/
    };

    std::variant<Text, Sprite, Geometry> renderData;
    int drawOrder = -1;
    TextureMods mods;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
};