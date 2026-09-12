#include "AssetViewerCommon.h"
#include <boost/pfr.hpp>
#include <ranges>
#include "../../../atlas/NewTextureRepository.h"
#include "../../../file/FilePathUtility.h"

namespace ui {

namespace {

constexpr std::array kIconResourcePaths{
	"ui/editor/file_folder_open_icon_small.png",
	"ui/editor/file_folder_open_icon_large.png",
	"ui/editor/file_folder_closed_icon_small.png",
	"ui/editor/file_folder_closed_icon_large.png",
	"ui/editor/image_file_icon_small.png",
	"ui/editor/audio_file_icon_small.png",
	"ui/editor/font_file_icon_small.png",
	"ui/editor/script_file_icon_small.png",
	"ui/editor/unknown_file_icon_small.png",
	"ui/editor/media_folder_icon_large.png"
};

template <std::size_t...Is>
bool IsLoadedImpl(const AssetViewerIcons& icons, std::index_sequence<Is...>)
{
	return ((boost::pfr::get<Is>(icons).resourceHandle.IsValid()) && ...);
}

} // unnamed

bool AssetViewerIcons::IsLoaded() const
{
	return IsLoadedImpl(*this, 
		std::make_index_sequence<boost::pfr::tuple_size_v<AssetViewerIcons>>{});
}

Result<AssetViewerIcons> AssetViewerIcons::Load(SDL_Renderer* renderer, SpriteAtlas& atlas)
{
	assert(renderer);

	/*static_assert(boost::pfr::tuple_size_v<AssetViewerIcons> == kIconResourcePaths.size());

	AssetViewerIcons icons{};
	Error err{};

	boost::pfr::for_each_field(icons, [&err, &atlas, renderer](auto& icon, size_t i) {
		if (!err.GetMessage().empty())
		{
			return;
		}
		auto pathResult = ResourcePath::Sprite(kIconResourcePaths[i]);
		if (!pathResult.Success())
		{
			err = std::move(pathResult).GetError();
			return;
		}
		auto loadResult = atlas.LoadSprite(
			renderer, { .filepath = std::move(pathResult).GetValue() });
		if (!loadResult.Success())
		{
			err = std::move(loadResult).GetError();
			return;
		}
		icon = std::move(loadResult).GetValue();
	});

	if (!err.GetMessage().empty())
	{
		return err;
	}

	return icons;*/

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

	return icons;
}

} // ui