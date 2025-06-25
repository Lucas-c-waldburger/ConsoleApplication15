#pragma once
#include "../LuaTypesRegistry.h"
#include "../../components/RenderableComponent.h"

// RENDER PROFILE
template <> static void RegisterLuaUserType<SDL_RendererFlip>(sol::state& lua)
{
	if (!lua["SDL_RendererFlip"].valid())
	{
		lua.new_enum("SDL_RendererFlip",
			"SDL_FLIP_NONE", SDL_FLIP_NONE,
			"SDL_FLIP_HORIZONTAL", SDL_FLIP_HORIZONTAL,
			"SDL_FLIP_VERTICAL", SDL_FLIP_VERTICAL);
	}
}

template <> static void RegisterLuaUserType<SDL_BlendMode>(sol::state& lua)
{
	if (!lua["SDL_BlendMode"].valid())
	{
		lua.new_enum("SDL_BlendMode",
			"SDL_BLENDMODE_NONE", SDL_BLENDMODE_NONE,
			"SDL_BLENDMODE_BLEND", SDL_BLENDMODE_BLEND,
			"SDL_BLENDMODE_ADD", SDL_BLENDMODE_ADD,
			"SDL_BLENDMODE_MOD", SDL_BLENDMODE_MOD,
			"SDL_BLENDMODE_MUL", SDL_BLENDMODE_MUL,
			"SDL_BLENDMODE_INVALID", SDL_BLENDMODE_INVALID);
	}
}

template <> static void RegisterLuaUserType<TextureMods>(sol::state& lua)
{
	if (!lua["TextureMods"].valid())
	{
		lua.new_usertype<TextureMods>("TextureMods",
			"color", &TextureMods::color,
			"blend", &TextureMods::blend);
	}
}

template <> static void RegisterLuaUserType<DebugDraw>(sol::state& lua)
{
	if (!lua["DebugDraw"].valid())
	{
		lua.new_usertype<DebugDraw>("DebugDraw", "on", &DebugDraw::on, "color", &DebugDraw::color);
	}
}

template <> static void RegisterLuaUserType<DebugDrawSet>(sol::state& lua)
{
	if (!lua["DebugDrawSet"].valid())
	{
		lua.new_usertype<DebugDrawSet>("DebugDrawSet",
			"boundingBox", &DebugDrawSet::boundingBox,
			"collider", &DebugDrawSet::collider);
	}
}

template <> static void RegisterLuaUserType<RenderProfile>(sol::state& lua)
{
	if (!lua["RenderProfile"].valid())
	{
		lua.new_usertype<RenderProfile>("RenderProfile",
			"drawOrder", &RenderProfile::drawOrder,
			"mods", &RenderProfile::mods,
			"flip", &RenderProfile::flip,
			"offset", &RenderProfile::offset,
			"debugDraw", &RenderProfile::debugDraw);
	}
}

// TEXT RENDERABLE
template <> static void RegisterLuaUserType<TextAlign>(sol::state& lua)
{
	if (!lua["TextAlign"].valid())
	{
		lua.new_enum("TextAlign",
			"Left", TextAlign::Left,
			"Right", TextAlign::Right,
			"Center", TextAlign::Center);
	}
}

template <> static void RegisterLuaUserType<GlyphInfo>(sol::state& lua)
{
	if (!lua["GlyphInfo"].valid())
	{
		lua.new_usertype<GlyphInfo>("GlyphInfo",
			"character", &GlyphInfo::character,
			"atlasRect", &GlyphInfo::atlasRect,
			"advance", &GlyphInfo::advance);
	}
}

template <> static void RegisterLuaUserType<GlyphCacheData>(sol::state& lua)
{
	if (!lua["GlyphCacheData"].valid())
	{
		lua.new_usertype<GlyphCacheData>("GlyphCacheData",
			"glyphInfo", &GlyphCacheData::glyphInfo,
			"destRect", &GlyphCacheData::destRect,
			"rotationCenter", &GlyphCacheData::rotationCenter);
	}
}

template <> static void RegisterLuaUserType<Handle<GlyphAtlas>>(sol::state& lua)
{
	if (!lua["Handle<GlyphAtlas>"].valid())
	{
		lua.new_usertype<Handle<GlyphAtlas>>("Handle<GlyphAtlas>",
			sol::meta_function::equal_to, &Handle<GlyphAtlas>::operator==);

		lua["Handle<GlyphAtlas>"]["__ne"] = 
			[](const Handle<GlyphAtlas>& lhs, const Handle<GlyphAtlas>& rhs) {
				return lhs != rhs;
		};
	}
}

template <> static void RegisterLuaUserType<TextRenderable>(sol::state& lua)
{
	if (!lua["DirtyFlag"].valid())
	{
		lua.new_enum("DirtyFlag",
			"NewText", TextRenderable::DirtyFlag::NewText,
			"NewTransforms", TextRenderable::DirtyFlag::NewTransforms);
	}
	if (!lua["TextRenderable"].valid())
	{
		lua.new_usertype<TextRenderable>("TextRenderable",
			"sourceAtlas", &TextRenderable::sourceAtlas,
			"text", &TextRenderable::text,
			"dimensions", &TextRenderable::dimensions,
			"align", &TextRenderable::align,
			"glyphCache", &TextRenderable::glyphCache,
			"dirtyFlags", &TextRenderable::dirtyFlags);
	}
}

// SPRITE RENDERABLE
template <> static void RegisterLuaUserType<Handle<SpriteSeriesAtlas>>(sol::state& lua)
{
	if (!lua["Handle<SpriteSeriesAtlas>"].valid())
	{
		lua.new_usertype<Handle<SpriteSeriesAtlas>>("Handle<SpriteSeriesAtlas>",
			sol::meta_function::equal_to, &Handle<SpriteSeriesAtlas>::operator==);

		lua["Handle<SpriteSeriesAtlas>"]["__ne"] =
			[](const Handle<SpriteSeriesAtlas>& lhs, const Handle<SpriteSeriesAtlas>& rhs) {
				return lhs != rhs;
		};
	}
}

template <> static void RegisterLuaUserType<SpriteRenderable>(sol::state& lua)
{
	if (!lua["SpriteRenderable"].valid())
	{
		lua.new_usertype<SpriteRenderable>("SpriteRenderable",
			"sourceAtlas", &SpriteRenderable::sourceAtlas,
			"sourcePlot", &SpriteRenderable::sourcePlot);
	}
}

// NEW RENDERABLE COMPONENT
template <> static void RegisterLuaUserType<NewRenderable>(sol::state& lua)
{
	if (!lua["NewRenderable"].valid())
	{
		lua.new_usertype<NewRenderable>("NewRenderable",
			"renderData", &NewRenderable::renderData,
			"profile", &NewRenderable::profile);
	}
}


namespace lua::usergroup {

//using RenderableUsergroup = TypeList<
//	SDL_BlendMode, 
//	SDL_RendererFlip, 
//	TextureMods,
//	DebugDraw,
//	DebugDrawSet,
//	RenderProfile,
//	TextAlign,
//	GlyphInfo,
//	Handle<GlyphAtlas>,
//	TextRenderable,
//	Handle<SpriteSeriesAtlas>,
//	SpriteRenderable,
//	NewRenderable
//>;

} // lua::usergroup