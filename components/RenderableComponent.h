#pragma once
#include "BaseComponent.h"
#include "../core/Handle.h"
#include "../physics/B2Shape.h"
#include "../atlas/SpriteSeriesAtlas.h"
#include "../atlas/GlyphAtlas.h"
#include "../core/TransparentStringHash.h"
#include "../core/Literals.h"
#include "../sdl/SDLite.h"
#include <variant>
#include <SDL.h>


// TODO: make draw order layers (overlay, etc.)

struct TextureModsOld
{
    uint8_t alpha = 255;
    struct RGB {
        uint8_t r = 255;
        uint8_t g = 255;
        uint8_t b = 255;

        friend constexpr bool operator==(const RGB& lhs, const RGB& rhs) = default;
    } color;
    SDL_BlendMode blend = SDL_BLENDMODE_BLEND;

    friend constexpr bool operator==(const TextureModsOld& lhs, const TextureModsOld& rhs) = default; 
};

constexpr bool operator==(const SDL_Color& lhs, const SDL_Color& rhs)
{
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

struct TextureMods
{
    struct RGB 
    { 
        uint8_t r = 255, g = 255, b = 255; 
        friend constexpr bool operator==(const RGB& lhs, const RGB& rhs)
        {
            return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
        }
    };

    RGB color = {};
    uint8_t alpha = 255;
    SDL_BlendMode blend = SDL_BLENDMODE_BLEND;

    friend constexpr bool operator==(const TextureMods& lhs, const TextureMods& rhs)
    {
        return lhs.color == rhs.color && lhs.blend == rhs.blend;
    }
};

struct DebugDraw
{
    bool on;
    SDL_Color color;
};
struct DebugDrawSet
{
    DebugDraw boundingBox = { .on = true, .color = SDLite::kColorRed };
    DebugDraw collider = { .on = true, .color = SDLite::kColorBlue };
};

struct RenderProfile
{
    int drawOrder = 0;
    TextureMods mods;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    SDL_FPoint offset = { 0.0f, 0.0f };
    DebugDrawSet debugDraw;
};

enum class TextAlign
{
    Left = 1,
    Center,
    Right
};

// should recompute if:
    // text context has changed
    // dimensions have changed
    // transform.scale has changed
    // align has changed
    // rotation has changed
struct GlyphCacheData
{
    Glyph glyph; // <- recompute only if text changed
    SDL_Rect destRect = { 0, 0, 0, 0 };  // <- recompute if text or transforms changed (align, dimensions, scale, etc)
    SDL_Point rotationCenter = { 0, 0 }; // <- recompute if text, transforms, or rotation changed
};

struct TextRenderable
{
    using GlyphCache = std::vector<GlyphCacheData>;

    Handle<GlyphAtlas> sourceAtlas;
    std::string text;
    Dimensions<int> dimensions = { 0, 0 };
    TextAlign align = TextAlign::Left;
    GlyphCache glyphCache;

    enum Flag : uint8_t
    {
       DirtyText = 1 << 0,
       DirtyTransform = 1 << 1,
       FixedSize = 1 << 2
       //NewRotation = 1 << 2,
       //All = (NewText | NewTransforms | NewRotation)
    };

    uint8_t flags = Flag::DirtyText;
};

struct SpriteRenderable 
{
    Handle<SpriteSeriesAtlas> sourceAtlas; // "if i give you this, and you say you're the same generation 
    AtlasPlot sourcePlot;                   // that plotted the atlas, then just render these rect coordinates
                                           // on your texture without additional checks"
};

struct Renderable : BaseComponent<Renderable>
{
    using RenderData = std::variant<std::monostate, SpriteRenderable, TextRenderable>;

    RenderData renderData;
    RenderProfile profile;
};


