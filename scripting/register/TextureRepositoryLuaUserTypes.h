#pragma once
#include "UserTypesCommon.h"
#include "../../atlas/NewTextureRepository.h"

DEF_REGISTER_LUA_USERTYPE(TextureRepository, "TextureRepository",
	"getSprite", [](const TextureRepository& repo, std::string name) {
		return repo.GetSpriteAtlas().GetSprite(name); },
	"getTextWriter", [](const TextureRepository& repo, std::string font) {
		return repo.GetFontAtlas().GetTextWriter(font); });