#include "AssetItem.h"

namespace ui {

AssetItem::Type GetAssetItemTypeFromPath(const std::filesystem::path& path)
{
	auto ext = path.extension().string();

	if (std::filesystem::is_directory(path))
	{
		return AssetItem::Type::Directory;
	}
	else if (std::filesystem::is_regular_file(path) && path.has_extension())
	{
		const auto ext = path.extension().string();

		if (kImageExtensions.contains(ext))
		{
			return AssetItem::Type::Image;
		}
		if (kFontExtensions.contains(ext))
		{
			return AssetItem::Type::Font;
		}
		if (kAudioExtensions.contains(ext))
		{
			return AssetItem::Type::Audio;
		}
		if (kScriptExtensions.contains(ext))
		{
			return AssetItem::Type::Script;
		}
	}

	return AssetItem::Type::Unknown;
}






} // ui