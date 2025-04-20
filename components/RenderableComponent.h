#pragma once
#include "BaseComponent.h"
#include "../core/Handle.h"
#include <variant>
#include <SDL.h>

class GlyphAtlas;
class SpriteSeriesAtlas;

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
    SDL_RendererFlip flip = SDL_FLIP_NONE;
};