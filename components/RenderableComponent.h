#pragma once
#include "BaseComponent.h"
#include "../core/Handle.h"
#include "../core/Hash.h"
#include "../atlas/SpriteAtlasCollection.h"
#include "../atlas/GlyphAtlasCollection.h"
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
    DebugDraw boundingBox = { .on = false, .color = SDLite::kColorRed };
    DebugDraw collider = { .on = false, .color = SDLite::kColorBlue };
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
        Handle<TextureResource> resourceHandle;
        HashType textHash = 0;
    };

    std::vector<GlyphCacheData> cache;
    CacheContext context;
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


