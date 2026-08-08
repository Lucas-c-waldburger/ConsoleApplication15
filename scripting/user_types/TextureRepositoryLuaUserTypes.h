#pragma once
#include "RenderableLuaUserTypes.h"
#include "../../atlas/NewTextureRepository.h"

DEF_LUA_USERTYPE(TextureRepository, Dependencies<Sprite, GlyphTextWriter>) {
	lua.def_type("getSprite", [](const TextureRepository& repo, std::string name) { 
								return repo.GetSpriteAtlas().GetSprite(name); },
				 "getTextWriter", [](const TextureRepository& repo, std::string font) {
									return repo.GetFontAtlas().GetTextWriter(font); });
}