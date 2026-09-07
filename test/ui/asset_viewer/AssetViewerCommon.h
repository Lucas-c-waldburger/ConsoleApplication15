#pragma once

#include <unordered_set>
#include <string_view>
#include "../../../atlas/SpriteAtlasCollection.h"

namespace ui {

static inline const std::unordered_set<std::string_view> kImageExtensions = {
".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif", ".webp", ".svg"
};
static inline const std::unordered_set<std::string_view> kAudioExtensions = {
	".mp3", ".ogg", ".flac", ".wav"
};
static inline const std::unordered_set<std::string_view> kFontExtensions = {
	".ttf"
};
static inline const std::unordered_set<std::string_view> kScriptExtensions = {
	".lua"
};

static inline constexpr std::string_view kDirectoryPayloadName = "ASSET_DIRECTORY";
static inline constexpr std::string_view kImagePayloadName = "ASSET_IMAGE";
static inline constexpr std::string_view kFontPayloadName = "ASSET_FONT";
static inline constexpr std::string_view kAudioPayloadName = "ASSET_AUDIO";
static inline constexpr std::string_view kScriptPayloadName = "ASSET_SCRIPT";

struct AssetViewerIcons
{
	bool IsLoaded() const;

	static Result<AssetViewerIcons> Load(SDL_Renderer* renderer, SpriteAtlas& atlas);

	Sprite folderClosedSmallSprite;
	Sprite folderClosedLargeSprite;
	Sprite folderOpenSmallSprite;
	Sprite folderOpenLargeSprite;
	Sprite imageFileSmallSprite;
	Sprite audioFileSmallSprite;
	Sprite fontFileSmallSprite;
	Sprite scriptFileSmallSprite;
	Sprite unknownFileSmallSprite;
};

} // ui