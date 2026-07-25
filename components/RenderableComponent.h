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

    friend constexpr bool operator==(const DebugDraw& lhs, const DebugDraw& rhs) noexcept
    {
        return lhs.on == rhs.on && lhs.color == rhs.color;
    }
};
struct DebugDrawSet
{
    DebugDraw boundingBox = { .on = false, .color = SDLite::kColorRed };
    DebugDraw collider = { .on = false, .color = SDLite::kColorBlue };

    friend constexpr bool operator==(const DebugDrawSet& lhs, const DebugDrawSet& rhs) noexcept
    {
        return lhs.boundingBox == rhs.boundingBox && lhs.collider == rhs.collider;
    }
};

struct RenderProfile
{
    struct Anchors
    {
        Anchor scale = Anchor::Center;
        Anchor rotation = Anchor::Center;

        friend constexpr bool operator==(const Anchors& lhs, const Anchors& rhs) noexcept
        {
            return lhs.scale == rhs.scale && lhs.rotation == rhs.rotation;
        }
    };

    int drawOrder = 0;
    TextureMods mods;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    SDL_FPoint offset = { 0.0f, 0.0f };
    DebugDrawSet debugDraw;
    bool isOverlay = false;
    float parallaxFactor = 0.0f;
    Anchors anchor;

    friend constexpr bool operator==(const RenderProfile& lhs, const RenderProfile& rhs) noexcept
    {
        return lhs.drawOrder == rhs.drawOrder && lhs.mods == rhs.mods && 
               lhs.flip == rhs.flip && lhs.offset == rhs.offset && lhs.debugDraw == rhs.debugDraw &&
               lhs.isOverlay == rhs.isOverlay && lhs.parallaxFactor == rhs.parallaxFactor &&
               lhs.anchor == rhs.anchor;
    }
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
    SDL_FRect destRect = { 0.0f, 0.0f, 0.0f, 0.0f };
    SDL_FPoint rotationCenter = { 0.0f, 0.0f }; 

    friend constexpr bool operator==(const GlyphCacheData& lhs, const GlyphCacheData& rhs) noexcept
    {
        return lhs.glyph == rhs.glyph && lhs.destRect == rhs.destRect && 
               lhs.rotationCenter == rhs.rotationCenter;
    }
};

struct TextFormatting
{
    Dimensions<int> bounds = { 1, 1 };
    TextAlign align = TextAlign::Left;
    float letterSpacing = 1.0f;
    bool scaleToBounds = true;

    friend constexpr bool operator==(const TextFormatting& lhs, const TextFormatting& rhs) noexcept
    {
        return lhs.bounds == rhs.bounds && lhs.align == rhs.align &&
               lhs.letterSpacing == rhs.letterSpacing && lhs.scaleToBounds == rhs.scaleToBounds;
    }
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

        friend constexpr bool operator==(const CacheContext& lhs, const CacheContext& rhs)
        {
            return lhs.transform == rhs.transform && lhs.formatting == rhs.formatting &&
                   lhs.offset == rhs.offset && lhs.resourceHandle == rhs.resourceHandle &&
                   lhs.textHash == rhs.textHash;
        }
    };

    std::vector<GlyphCacheData> cache;
    CacheContext context;

    friend bool operator==(const TextRenderableGlyphCache& lhs, const TextRenderableGlyphCache& rhs)
    {
        return lhs.cache == rhs.cache && lhs.context == rhs.context;
    }
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

    bool operator==(const TextRenderableComponent&) const = default;
};

struct SpriteRenderableComponent : BaseComponent<SpriteRenderableComponent>
{
    Sprite sprite;
    RenderProfile profile;

    bool operator==(const SpriteRenderableComponent&) const = default;
};


