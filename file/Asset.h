#pragma once
#include <cstdint>
#include <filesystem>
#include <string_view>
#include <vector>
#include "../core/Result.h"
#include "../core/SizedEnum.h"
#include "../atlas/SpriteSeriesAtlas.h"
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <map>

namespace fs = std::filesystem;

//inline Result<SpriteSeriesResourcePacket> 
//LoadSprites(const std::string& directoryPath, 
//			std::initializer_list<std::string> filenames)
//{
//	auto dirPath = fs::path{ directoryPath };
//	if (!fs::exists(dirPath))
//	{
//		return MAKE_ERROR_FMT("Directory path '{}' does not exist", directoryPath);
//	}
//
//	SpriteSeriesMetadata metadata{ .seriesName = dirPath.stem().string() };
//
//
//}

//inline Result<SpriteSeriesResourcePacket> LoadSingleSprite(const std::string& directoryPath)
//{
//
//}

inline Result<Void> LoadSpriteDirectory(const std::string& directoryPath,
								 SpriteSeriesResourcePackets& allPackets)
{
	auto dirPath = fs::path{ directoryPath };
	if (!fs::exists(dirPath))
	{
		return MAKE_ERROR_FMT("Directory path '{}' does not exist", directoryPath);
	}

	SpriteSeriesMetadata metadata{ .seriesName = dirPath.stem().string() };
	std::map<size_t, std::string, std::less<>> spriteIdxToPaths;

	for (const auto& entry : fs::directory_iterator(dirPath))
	{
		auto entryPath = entry.path();

		if (entry.is_directory())
		{
			TRY(LoadSpriteDirectory(entryPath.string(), allPackets));
			continue;
		}

		if (entry.is_regular_file())
		{
			std::string stem = entryPath.stem().string();
			auto stemView = std::string_view{ stem };
			
			size_t idxSepPos = stemView.find_last_of('_');
			if (idxSepPos == std::string_view::npos)
			{
				return MAKE_ERROR_FMT("Invalid sprite file stem format '{}'", stemView);
			}

			auto idxStr = std::string{ stemView.substr(idxSepPos + 1) };
			int idx = -1;
			
			try {
				idx = std::stoi(idxStr);
			}
			catch (std::exception& ex) {
				return MAKE_ERROR(ex.what());
			}
			
			if (idx < 0)
			{
				return MAKE_ERROR_FMT("Invalid sprite index '{}'", idx);
			}

			auto [_, inserted] = spriteIdxToPaths.try_emplace(idx, entry.path().string());
			if (!inserted)
			{
				return MAKE_ERROR_FMT("Duplicate sprite index in series '{}'", idx);
			}
		}
	}

	if (spriteIdxToPaths.empty())
	{
		return Void{};
	}
	
	std::vector<std::string> paths;
	paths.reserve(spriteIdxToPaths.size());

	for (size_t i = 0; i < spriteIdxToPaths.size(); i++)
	{
		auto it = spriteIdxToPaths.find(i);
		if (it == spriteIdxToPaths.end())
		{
			return MAKE_ERROR_FMT("Sprite series had missing index '{}'", i);
		}

		paths.emplace_back(std::move(it->second));
	}

	auto& newPacket = allPackets.emplace_back();
	newPacket.SetMetadata(std::move(metadata));
	newPacket.SetFilepaths(std::move(paths));

	return Void{};
}

//enum class FileExtension : uint8_t {
//	PNG = 1 << 0,
//	JPG = 1 << 1,
//	WAV = 1 << 2,
//	TTF = 1 << 3
//};

//namespace fs = std::filesystem;
//
//enum class AssetType
//{
//	Invalid = -1,
//	Image,
//	Audio,
//	Font,
//	ENUM_SIZE_
//};
//static_assert(SomeSizedEnum<AssetType>);
//
//using ExtSet = std::unordered_set<std::string_view>;
//
//inline const std::array<ExtSet, enum_size_v<AssetType>>
//kAssetTypeFileExtensions = {
//	ExtSet{ ".png", ".jpg", ".jpeg", ".bmp" },
//	ExtSet{ ".wav", ".mp3" },
//	ExtSet{ ".ttf" }
//};
//
//inline AssetType GetAssetType(std::string_view ext)
//{
//	for (size_t i = 0; i < enum_size_v<AssetType>; i++)
//	{
//		if (kAssetTypeFileExtensions[i].contains(ext))
//		{
//			return static_cast<AssetType>(i);
//		}
//	}
//
//	return AssetType::Invalid;
//}
//
//struct Asset
//{
//	AssetType type = AssetType::Invalid;
//	std::string filepath;
//};
//
//struct SpriteAsset : Asset
//{
//	SpriteAsset() : Asset{ AssetType::Image } {}
//	std::string seriesName;
//	size_t seriesIndex = 0;
//};
//
//template <typename T>
//Result<std::vector<T>> LoadAssets(std::string_view);
////template <typename T>
////struct AssetLoader;
//
//template <typename T>
//concept SomeAsset = requires(std::string_view sv) {
//	std::derived_from<T, Asset>;
//	{ LoadAssets<T>(sv) } -> std::same_as<Result<std::vector<T>>>;
//};
//
//template <SomeAsset T>
//using AssetPack = std::vector<T>;
//
//struct SpriteSeriesAssetPack
//{
//	std::string seriesName;
//	std::vector<std::string> orderedPaths;
//};
//
////template <typename T>
////struct AssetLoadSpecification
//
//inline Result<SpriteSeriesAssetPack> LoadSpriteAssets(std::string_view folderPath,
//													   size_t reserveSize = 12)
//{
//	SpriteSeriesAssetPack spriteAssets;
//	spriteAssets.orderedPaths.reserve(reserveSize);
//	//size_t largestIdx = 0;
//
//	std::string seriesName;
//	
//	for (const auto& entry : fs::directory_iterator(folderPath)) 
//	{
//		if (!entry.is_regular_file()) 
//		{
//			continue;
//		}
//
//		auto path = entry.path();
//		std::string ext = path.extension().string();
//
//		if (GetAssetType(ext) != AssetType::Image)
//		{
//			return MAKE_ERROR_FMT("Invalid file extension '{}'", ext);
//		}
//
//		std::string stem = path.stem().string();
//		auto stemView = std::string_view{ stem };
//
//		size_t nameSepPos = stemView.find_last_of('_');
//		if (nameSepPos == std::string_view::npos)
//		{
//			return MAKE_ERROR_FMT("Invalid sprite file stem format '{}'", stemView);
//		}
//
//		auto foundName = stemView.substr(0, nameSepPos);
//		if (seriesName.empty())
//		{
//			seriesName = foundName;
//		}
//		else if (foundName != seriesName)
//		{
//			return MAKE_ERROR_FMT("Series names inconsistent: '{}' vs. '{}'", 
//				seriesName, foundName);
//		}
//
//		SpriteAsset newAsset{};
//		newAsset.seriesName = seriesName;
//
//		auto idxStr = std::string{ stemView.substr(nameSepPos + 1) };
//		int idx = -1;
//
//		try {
//			idx = std::stoi(idxStr);
//		}
//		catch (std::exception& ex) {
//			return MAKE_ERROR(ex.what());
//		}
//
//		if (idx < 0)
//		{
//			return MAKE_ERROR_FMT("Invalid sprite index '{}'", idx);
//		}
//
//		newAsset.seriesIndex = static_cast<size_t>(idx);
//		if (newAsset.seriesIndex >= spriteAssets.size())
//		{
//			spriteAssets.resize(newAsset.seriesIndex + 1);
//		}
//
//		largestIdx = std::max(largestIdx, newAsset.seriesIndex);
//
//		spriteAssets[newAsset.seriesIndex] = std::move(newAsset);
//	}
//
//	if (largestIdx + 1 < spriteAssets.size())
//	{
//		spriteAssets.resize(largestIdx + 1);
//	}
//
//	return spriteAssets;
//}


//class AssetPack
//{
//public:
//	static AssetPack LoadSpriteAssets(std::string_view folderPath)
//	{
//
//	}
//
//private:
//	std::string folder_;
//	std::vector<Asset> assets_;
//};

//struct SpriteAsset : Asset
//{
//	SpriteAsset() : Asset{ AssetType::Image, "" } {}
//
//
//};