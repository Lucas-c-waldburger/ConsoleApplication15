#pragma once
#include "BaseComponent.h"
#include "../core/Handle.h"
#include "../atlas/SpriteSeriesAtlas.h"
#include "../atlas/SpriteAtlas.h"
#include "../atlas/GlyphAtlas.h"
#include "../atlas/NewGlyphAtlas.h"
#include "../sdl/SDLite.h"
#include "../sdl/SDLUtils.h"
#include "../render/TextureMods.h"
#include "../core/Anchor.h"
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
    struct Anchors
    {
        Anchor scale = Anchor::Center;
        Anchor rotation = Anchor::Center;
    };

    int drawOrder = 0;
    TextureMods mods;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    SDL_FPoint offset = { 0.0f, 0.0f };
    DebugDrawSet debugDraw;
    bool isOverlay = false;
    float parallaxFactor = 0.0f;
    Anchors anchor;
};

enum class TextAlign
{
    Left = 1,
    Center,
    Right
};

struct GlyphCacheData
{
    Glyph glyph;
    SDL_Rect destRect = { 0, 0, 0, 0 };
    SDL_Point rotationCenter = { 0, 0 }; 
};

struct TextFormatting
{
    Dimensions<int> bounds = { 1, 1 };
    TextAlign align = TextAlign::Left;
    float letterSpacing = 1.0f;
    bool scaleToBounds = true;

    bool operator==(const TextFormatting& rhs) const = default;
};

struct TextRenderableGlyphCache : BaseComponent<TextRenderableGlyphCache>
{
    struct CacheContext
    {
        Transform transform;
        TextFormatting formatting;
        SDL_FPoint offset = { 0.0f, 0.0f };
        Handle<NewTextureAtlas> sourceAtlas;
    };

    std::vector<GlyphCacheData> cache;
    CacheContext context;
};

struct NewTextRenderable
{
    GlyphTextWriter writer;
    TextFormatting formatting;
};

struct NewSpriteRenderable
{
    Sprite sprite;
};

inline constexpr RenderProfile kDefaultTextRenderProfile{
    .mods = { .color = {0, 0, 0} },
    .isOverlay = true,
};

struct TextRenderableComponent : BaseComponent<TextRenderableComponent>
{
    GlyphTextWriter writer;
    TextFormatting formatting;
    RenderProfile profile = kDefaultTextRenderProfile;
};

struct SpriteRenderableComponent : BaseComponent<SpriteRenderableComponent>
{
    Sprite sprite;
    RenderProfile profile;
};


template <typename T>
concept SomeRenderableComponent = std::same_as<T, NewTextRenderable> ||
                                  std::same_as<T, NewSpriteRenderable>;

using NewRenderableVariant = std::variant<NewTextRenderable, NewSpriteRenderable>;

struct NewRenderable : BaseComponent<NewRenderable>
{
    NewRenderableVariant renderData;
    RenderProfile profile;
    /* -------------------- */
    struct {
        Handle<NewTextureAtlas> sourceAtlas;
        size_t renderCallCount = 0;
    } internals_;
};

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


