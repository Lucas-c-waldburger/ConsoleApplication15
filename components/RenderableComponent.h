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
    SDL_Color color = { 0, 0, 0, 255 };
    SDL_BlendMode blend = SDL_BLENDMODE_BLEND;

    friend constexpr bool operator==(const TextureMods& lhs, const TextureMods& rhs)
    {
        return lhs.color == rhs.color && lhs.blend == rhs.blend;
    }
};


//struct Renderable : BaseComponent<Renderable, 7>
struct Renderable : BaseComponent<Renderable>
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
    TextureModsOld mods;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
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
    GlyphInfo glyphInfo; // <- recompute only if text changed
    SDL_Rect destRect = { 0, 0, 0, 0 };  // <- recompute if text or transforms changed (align, dimensions, scale, etc)
    SDL_Point rotationCenter = { 0, 0 }; // <- recompute if text, transforms, or rotation changed
};

struct TextRenderable
{
    using GlyphCache = std::vector<GlyphCacheData>;

    Handle<GlyphAtlas> sourceAtlas;
    std::string text;
    Dimensions<int> dimensions = { 0, 0 };
    TextAlign align;
    GlyphCache glyphCache;

    enum DirtyFlag : uint8_t
    {
       NewText = 1 << 0,
       NewTransforms = 1 << 1,
       //NewRotation = 1 << 2,
       //All = (NewText | NewTransforms | NewRotation)
    };

    uint8_t dirtyFlags = DirtyFlag::NewText;
};

struct SpriteRenderable 
{
    Handle<SpriteSeriesAtlas> sourceAtlas; // "if i give you this, and you say you're the same generation 
    SDL_Rect sourcePlot;                   // that plotted the atlas, then just render these rect coordinates
                                           // on your texture without additional checks"
};

//struct NewRenderable : BaseComponent<NewRenderable, 14>
struct NewRenderable : BaseComponent<NewRenderable>
{
    std::variant<SpriteRenderable, TextRenderable> renderData;
    RenderProfile profile;
};


