#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include "AssetViewerCommon.h"

namespace ui {

struct AssetItem
{
	enum Type
	{
		Unknown,
		Directory,
		Image,
		Font,
		Audio,
		Script
	};

	std::filesystem::path path;
	std::string displayName;
	std::vector<size_t> fileTypeChildren;
	Type type = Type::Unknown;

	constexpr bool IsFile() const noexcept
	{
		return type != AssetItem::Type::Directory &&
			   type != AssetItem::Type::Unknown;
	}

	operator bool() const
	{
		return type != AssetItem::Type::Unknown && !path.empty();
	}
	bool operator!() const { return !(*this); }
};

AssetItem::Type GetAssetItemTypeFromPath(const std::filesystem::path& path);

constexpr std::string_view GetAssetItemPayloadName(const AssetItem::Type type)
{
	switch (type)
	{
	case AssetItem::Type::Directory: return kDirectoryPayloadName;
	case AssetItem::Type::Image: return kImagePayloadName;
	case AssetItem::Type::Font: return kFontPayloadName;
	case AssetItem::Type::Audio: return kAudioPayloadName;
	case AssetItem::Type::Script: return kScriptPayloadName;
	}

	return {};
}



} // ui