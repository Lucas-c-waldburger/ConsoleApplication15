#pragma once
#include "BaseComponent.h"
#include "../core/Handle.h"
#include "../physics/B2Shape.h"
#include <variant>
#include <SDL.h>

class GlyphAtlas;
class SpriteSeriesAtlas;

// TODO: make draw order layers (overlay, etc.)

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
    };

    std::variant<Text, Sprite, Geometry> renderData;
    int drawOrder = -1;
    uint8_t opacity = 255;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
};