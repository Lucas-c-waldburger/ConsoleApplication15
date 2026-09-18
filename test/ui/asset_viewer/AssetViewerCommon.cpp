#include "AssetViewerCommon.h"
#include "../../../atlas/NewTextureRepository.h"
#include "../../../file/FilePathUtility.h"

namespace ui {

Result<AssetViewerIcons> AssetViewerIcons::Load(SDL_Renderer* renderer, SpriteAtlas& atlas)
{
	assert(renderer);

	TRY(ResourcePath::Sprite("ui/editor/file_folder_open_icon_small.png"),
		folderOpenSmallPath);
	TRY(ResourcePath::Sprite("ui/editor/file_folder_open_icon_large.png"),
		folderOpenLargePath);
	TRY(ResourcePath::Sprite("ui/editor/file_folder_closed_icon_small.png"),
		folderClosedSmallPath);
	TRY(ResourcePath::Sprite("ui/editor/file_folder_closed_icon_large.png"),
		folderClosedLargePath);
	TRY(ResourcePath::Sprite("ui/editor/image_file_icon_small.png"),
		imageFileSmallPath);
	TRY(ResourcePath::Sprite("ui/editor/audio_file_icon_small.png"),
		audioFileSmallPath);
	TRY(ResourcePath::Sprite("ui/editor/font_file_icon_small.png"),
		fontFileSmallPath);
	TRY(ResourcePath::Sprite("ui/editor/script_file_icon_small.png"),
		scriptFileSmallPath);
	TRY(ResourcePath::Sprite("ui/editor/unknown_file_icon_small.png"),
		unknownFileSmallPath);
	TRY(ResourcePath::Sprite("ui/editor/media_folder_icon_large.png"),
		mediaFolderLargePath);
	TRY(ResourcePath::Sprite("ui/editor/music_file_icon_large.png"),
		musicFileLargePath);
	TRY(ResourcePath::Sprite("ui/editor/sound_file_icon_large.png"),
		soundFileLargePath);
	TRY(ResourcePath::Sprite("ui/editor/font_file_icon_large.png"),
		fontFileLargePath);

	AssetViewerIcons icons{};

	TRY_ASSIGN(icons.folderOpenSmallSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(folderOpenSmallPath) }));
	TRY_ASSIGN(icons.folderOpenLargeSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(folderOpenLargePath) }));
	TRY_ASSIGN(icons.folderClosedSmallSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(folderClosedSmallPath) }));
	TRY_ASSIGN(icons.folderClosedLargeSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(folderClosedLargePath) }));
	TRY_ASSIGN(icons.imageFileSmallSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(imageFileSmallPath) }));
	TRY_ASSIGN(icons.audioFileSmallSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(audioFileSmallPath) }));
	TRY_ASSIGN(icons.fontFileSmallSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(fontFileSmallPath) }));
	TRY_ASSIGN(icons.scriptFileSmallSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(scriptFileSmallPath) }));
	TRY_ASSIGN(icons.unknownFileSmallSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(unknownFileSmallPath) }));
	TRY_ASSIGN(icons.mediaFolderLargeSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(mediaFolderLargePath) }));
	TRY_ASSIGN(icons.musicFileLargeSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(musicFileLargePath) }));
	TRY_ASSIGN(icons.soundFileLargeSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(soundFileLargePath) }));
	TRY_ASSIGN(icons.fontFileLargeSprite, atlas.LoadSprite(
		renderer, { .filepath = std::move(fontFileLargePath) }));

	return icons;
}

} // ui