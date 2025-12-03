#pragma once
#include <ranges>
#include "SDLJsonUserTypes.h"
#include "CoreJsonUserTypes.h"
#include "AtlasJsonUserTypes.h"
#include "../../components/RenderableComponent.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RGB, r, g, b)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextureMods, color, alpha, blend)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DebugDraw, on, color)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DebugDrawSet, boundingBox, collider)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RenderProfile::Anchors, scale, rotation)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RenderProfile, drawOrder, mods, flip, offset, debugDraw, 
								   isOverlay, parallaxFactor, anchor)

NLOHMANN_JSON_SERIALIZE_ENUM(
	TextAlign,
	{
		{TextAlign::Left,   "Left"},
		{TextAlign::Center, "Center"},
		{TextAlign::Right,  "Right"}
	}
)



NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Glyph, character, plot, advance)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GlyphCacheData, glyph, destRect, rotationCenter)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextFormatting, bounds, align, 
								   letterSpacing, scaleToBounds)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GlyphTextWriter, text)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextRenderableGlyphCache::CacheContext, transform, 
								   formatting, offset)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextRenderableGlyphCache, cache, context)

//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Sprite, sourceAtlas, plot, spriteIndex)



