#pragma once
#include "BaseComponent.h"
#include "../core/Handle.h"
#include "../atlas/SpriteSeriesAtlas.h"
#include "../atlas/GlyphAtlas.h"
#include "../sdl/SDLite.h"
#include "../sdl/SDLUtils.h"
#include "../render/TextureMods.h"
#include "TransformComponent.h"
#include <variant>


// TODO: make draw order layers (overlay, etc.)

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

struct TextFormatting
{
    Dimensions<int> bounds = { 0, 0 };
    TextAlign align = TextAlign::Left;
    float letterSpacing = 1.0f;
    bool scaleToBounds = true;

    bool operator==(const TextFormatting& rhs) const = default;
};

struct GlyphCache : BaseComponent<GlyphCache>
{
    struct CacheContext
    {
        Transform transform;
        TextFormatting format;
        SDL_FPoint offset = { 0.0f, 0.0f };
    };

    std::vector<GlyphCacheData> cache;
    CacheContext context;
};



struct NewTextRenderable
{
    std::string text;
    TextFormatting format;
};

struct NewSpriteRenderable
{
    int plotIndex = -1;
};

using NewRenderableVariant = std::variant<NewTextRenderable, NewSpriteRenderable>;

class NewTextureAtlas;

struct NewRenderable : BaseComponent<NewRenderable>
{
    Handle<NewTextureAtlas> sourceAtlas;
    NewRenderableVariant renderData;
    RenderProfile profile;
};

//
//struct Texture;
//struct NewSpriteRenderable
//{
//    Handle<Texture> sprite;
//};

//struct NewRenderable
//{
//    Handle<NewTextureAtlas> sourceAtlas;
//    RenderProfile profile;
//};

struct TextRenderable
{
    using GlyphCache = std::vector<GlyphCacheData>;

    Handle<GlyphAtlas> sourceAtlas;
    std::string text;
    Dimensions<int> dimensions = { 0, 0 };
    TextAlign align = TextAlign::Left;
    GlyphCache glyphCache;
    TextFormatting format;

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


