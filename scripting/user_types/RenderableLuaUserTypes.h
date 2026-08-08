#pragma once
#include "CoreLuaUserTypes.h"
#include "../../components/RenderableComponent.h"

// ATLAS
DEF_LUA_USERTYPE(AtlasPlot, Dependencies<SDL_Rect>) {
	lua.def_type("rect", &AtlasPlot::rect,
				 "rotation", &AtlasPlot::rotation);
}

// RENDER PROFILE
DEF_LUA_USERTYPE(SDL_RendererFlip) {
	lua.def_enum("SDL_FLIP_NONE", SDL_FLIP_NONE,
			     "SDL_FLIP_HORIZONTAL", SDL_FLIP_HORIZONTAL,
			     "SDL_FLIP_VERTICAL", SDL_FLIP_VERTICAL);
}

DEF_LUA_USERTYPE(SDL_BlendMode) {
	lua.def_enum("SDL_BLENDMODE_NONE", SDL_BLENDMODE_NONE,
				 "SDL_BLENDMODE_BLEND", SDL_BLENDMODE_BLEND,
				 "SDL_BLENDMODE_ADD", SDL_BLENDMODE_ADD,
				 "SDL_BLENDMODE_MOD", SDL_BLENDMODE_MOD,
				 "SDL_BLENDMODE_MUL", SDL_BLENDMODE_MUL,
				 "SDL_BLENDMODE_INVALID", SDL_BLENDMODE_INVALID);
}

DEF_LUA_USERTYPE(RGB) {
	lua.def_type("r", &RGB::r,
				 "g", &RGB::g,
				 "b", &RGB::b);
}

DEF_LUA_USERTYPE(TextureMods, Dependencies<RGB, SDL_BlendMode>) {
	lua.def_type("color", &TextureMods::color,
				 "alpha", &TextureMods::alpha,
				 "blend", &TextureMods::blend);
}

DEF_LUA_USERTYPE(DebugDraw, Dependencies<SDL_Color>) {
	lua.def_type("on", &DebugDraw::on, 
				 "color", &DebugDraw::color);
}

DEF_LUA_USERTYPE(DebugDrawSet, Dependencies<DebugDraw>) {
	lua.def_type("boundingBox", &DebugDrawSet::boundingBox,
				 "collider", &DebugDrawSet::collider);
}

DEF_LUA_USERTYPE(Anchor) {
	lua.def_enum("Left", Anchor::Left,
				 "Right", Anchor::Right,
				 "Top", Anchor::Top,
				 "Bottom", Anchor::Bottom,
		         "Center", Anchor::Center);
}

using RenderProfileAnchors = RenderProfile::Anchors;
DEF_LUA_USERTYPE(RenderProfileAnchors, Dependencies<Anchor>) {
	lua.def_type("scale", &RenderProfileAnchors::scale,
				 "rotation", &RenderProfileAnchors::rotation);
}

DEF_LUA_USERTYPE(RenderProfile, Dependencies<TextureMods, SDL_RendererFlip, SDL_FPoint, 
											 DebugDrawSet, RenderProfileAnchors>) {
	lua.def_type("drawOrder", &RenderProfile::drawOrder,
				 "mods", &RenderProfile::mods,
				 "flip", &RenderProfile::flip,
				 "offset", &RenderProfile::offset,
				 "debugDraw", &RenderProfile::debugDraw,
				 "isOverlay", &RenderProfile::isOverlay,
				 "parallaxFactor", &RenderProfile::parallaxFactor,
				 "anchors", &RenderProfile::anchor);
}

// TEXT RENDERABLE
DEF_LUA_USERTYPE(TextAlign) {
	lua.def_enum("Left", TextAlign::Left,
				 "Right", TextAlign::Right,
				 "Center", TextAlign::Center);
}

DEF_LUA_USERTYPE(Glyph, Dependencies<AtlasPlot>) {
	lua.def_type("character", &Glyph::character,
				 "plot", &Glyph::plot,
				 "advance", &Glyph::advance);
}

DEF_LUA_USERTYPE(GlyphCacheData, Dependencies<Glyph, SDL_Rect, SDL_FPoint>) {
	lua.def_type("glyph", &GlyphCacheData::glyph,
				 "destRect", &GlyphCacheData::destRect,
				 "rotationCenter", &GlyphCacheData::rotationCenter);
}

using TextureResourceHandle = Handle<TextureResource>;
DEF_LUA_USERTYPE(TextureResourceHandle) {
	lua.def_type(sol::meta_function::equal_to, &TextureResourceHandle::operator==);
}

DEF_LUA_USERTYPE(TextFormatting, Dependencies<IDimensions, TextAlign>) {
	lua.def_type("bounds", &TextFormatting::bounds,
				 "align", &TextFormatting::align,
				 "letterSpacing", &TextFormatting::letterSpacing,
				 "scaleToBounds", &TextFormatting::scaleToBounds);
}

DEF_LUA_USERTYPE(Sprite, Dependencies<TextureResourceHandle, AtlasPlot>) {
	lua.def_type("resourceHandle", &Sprite::resourceHandle,
				 "plot", &Sprite::plot);
}

DEF_LUA_USERTYPE(GlyphTextWriter, Dependencies<TextureResourceHandle>) {
	lua.def_type("resourceHandle", &GlyphTextWriter::resourceHandle,
				 "text", &GlyphTextWriter::text);
}

DEF_LUA_USERTYPE(TextRenderableComponent, Dependencies<GlyphTextWriter, TextFormatting, RenderProfile>) {

	static constexpr auto setFont = [](TextRenderableComponent& r, std::string font, 
									   sol::this_state st) {
		sol::state_view view{ st };
		sol::table eng = view["engine"];
		if (eng != sol::nil)
		{
			sol::table tx = eng["textures"];
			if (tx != sol::nil)
			{
				std::string tempText = std::move(r.writer.text);
				r.writer = tx["getTextWriter"](font);
				r.writer.text = std::move(tempText);

				if (!r.writer.resourceHandle.IsValid())
				{
					LOG_ERROR_FMT("Font '{}' not found");
				}
			}
		} 
	};

	lua.def_type("writer", &TextRenderableComponent::writer,
				 "setFont", setFont,
				 "formatting", &TextRenderableComponent::formatting,
				 "profile", &TextRenderableComponent::profile,
				 sol::meta_function::equal_to, &TextRenderableComponent::operator==);
}

DEF_LUA_USERTYPE(SpriteRenderableComponent, Dependencies<Sprite, RenderProfile>) {

	static constexpr auto setSprite = [](SpriteRenderableComponent& r, std::string name, 
										 sol::this_state st) {
		sol::state_view view{ st };
		sol::table eng = view["engine"];
		if (eng != sol::nil)
		{
			sol::table tx = eng["textures"];
			if (tx != sol::nil)
			{
				r.sprite = tx["getSprite"](name);
				if (!r.sprite.resourceHandle.IsValid())
				{
					LOG_ERROR_FMT("Sprite '{}' not found");
				}
			}
		}
	};

	lua.def_type("sprite", &SpriteRenderableComponent::sprite,
				 "profile", &SpriteRenderableComponent::profile,
				 "setSprite", setSprite,
				 sol::meta_function::equal_to, &SpriteRenderableComponent::operator==);
}