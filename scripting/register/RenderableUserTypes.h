#pragma once
#include "UserTypesCommon.h"
#include "CoreUserTypes.h"

#ifdef RGB
#undef RGB
#endif
#include "../../components/RenderableComponent.h"

// ATLAS
DEF_REGISTER_LUA_USERTYPE(AtlasPlot, "AtlasPlot",
	"rect", &AtlasPlot::rect,
	"rotation", &AtlasPlot::rotation);

// RENDER PROFILE
DEF_REGISTER_LUA_ENUM(SDL_RendererFlip, "RenderFlip");
DEF_REGISTER_LUA_ENUM(SDL_BlendMode, "BlendMode");

DEF_REGISTER_LUA_USERTYPE(RGB, "RGB", 
	"r", &RGB::r, "g", &RGB::g, "b", &RGB::b);

DEF_REGISTER_LUA_USERTYPE(TextureMods, "TextureMods",
	"color", &TextureMods::color,
	"alpha", &TextureMods::alpha,
	"blend", &TextureMods::blend);

DEF_REGISTER_LUA_USERTYPE(DebugDraw, "DebugDraw",
	"on", &DebugDraw::on,
	"color", &DebugDraw::color);

DEF_REGISTER_LUA_USERTYPE(DebugDrawSet, "DebugDrawSet",
	"boundingBox", &DebugDrawSet::boundingBox,
	"collider", &DebugDrawSet::collider);

DEF_REGISTER_LUA_ENUM(Anchor, "Anchor");

DEF_REGISTER_LUA_USERTYPE(RenderProfile::Anchors, "RenderProfileAnchors",
	"scale", &RenderProfile::Anchors::scale,
	"rotation", &RenderProfile::Anchors::rotation);

DEF_REGISTER_LUA_USERTYPE(RenderProfile, "RenderProfile",
	"drawOrder", &RenderProfile::drawOrder,
	"mods", &RenderProfile::mods,
	"flip", &RenderProfile::flip,
	"offset", &RenderProfile::offset,
	"debugDraw", &RenderProfile::debugDraw,
	"isOverlay", &RenderProfile::isOverlay,
	"parallaxFactor", &RenderProfile::parallaxFactor,
	"anchors", &RenderProfile::anchor);

// TEXT FORMATTING
DEF_REGISTER_LUA_ENUM(TextAlign, "TextAlign");

DEF_REGISTER_LUA_USERTYPE(Glyph, "Glyph",
	"character", &Glyph::character,
	"plot", &Glyph::plot,
	"advance", &Glyph::advance);

DEF_REGISTER_LUA_USERTYPE(GlyphCacheData, "GlyphCacheData",
	"glyph", &GlyphCacheData::glyph,
	"destRect", &GlyphCacheData::destRect,
	"rotationCenter", &GlyphCacheData::rotationCenter);

DEF_REGISTER_LUA_USERTYPE(Handle<TextureResource>, "TextureResourceHandle",
	sol::meta_function::equal_to, &Handle<TextureResource>::operator==);

DEF_REGISTER_LUA_USERTYPE(TextFormatting, "TextFormatting",
	"bounds", &TextFormatting::bounds,
	"align", &TextFormatting::align,
	"letterSpacing", &TextFormatting::letterSpacing,
	"scaleToBounds", &TextFormatting::scaleToBounds);

// COMPONENTS
DEF_REGISTER_LUA_USERTYPE(Sprite, "Sprite",
	"resourceHandle", &Sprite::resourceHandle,
	"plot", &Sprite::plot);

DEF_REGISTER_LUA_USERTYPE(GlyphTextWriter, "GlyphTextWriter",
	"resourceHandle", &GlyphTextWriter::resourceHandle,
	"text", &GlyphTextWriter::text);

DEF_REGISTER_LUA_USERTYPE(TextRenderableComponent, "TextRenderableComponent",
	"writer", &TextRenderableComponent::writer,
	"formatting", &TextRenderableComponent::formatting,
	"profile", &TextRenderableComponent::profile,
	sol::meta_function::equal_to, &TextRenderableComponent::operator==);

DEF_REGISTER_LUA_USERTYPE(SpriteRenderableComponent, "SpriteRenderableComponent",
	"sprite", &SpriteRenderableComponent::sprite,
	"profile", &SpriteRenderableComponent::profile,
	sol::meta_function::equal_to, &SpriteRenderableComponent::operator==);