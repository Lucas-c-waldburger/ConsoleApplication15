#pragma once
#include "Atlas.h"
#include "../core/commonObjects.h"
#include <unordered_map>

struct SpriteInfo
{
	int index = -1;
	SDL_Rect atlasRect = { 0, 0, 0, 0 };
};

struct SeriesInfo
{
	std::string seriesName;
	size_t size;

	bool operator==(const SeriesInfo&) const = default;
};

namespace std {
	template <>
	struct hash<SeriesInfo> {
		size_t operator()(const SeriesInfo& si) const noexcept {
			auto h1 = std::hash<std::string>{}(si.seriesName);
			auto h2 = std::hash<size_t>{}(si.size);
			return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
		}
	};
}

class SpriteSeriesAtlas;

template <>
struct AtlasInfo<SpriteSeriesAtlas>
{
	struct SeriesData
	{
		std::string seriesName;
		std::vector<std::string> spriteFilepaths; // its your job to have these ordered in the right index
	};

	std::vector<SeriesData> seriesDatas;
};

class SpriteSeriesAtlas : public Atlas<SpriteSeriesAtlas>
{
public:
	SpriteSeriesAtlas() = default;
	SpriteSeriesAtlas(const Handle<SpriteSeriesAtlas>& handle) : Atlas(handle) {}

	bool Load(SDL_Renderer* renderer, AtlasInfo args);
	SpriteInfo GetSprite(std::string_view seriesName, size_t index, bool wrapOnOutOfRange = true) const;

private:
	struct AtlasSizeAccumulator
	{
		int totalArea = 0;
		Dimensions<int> maxDims = { 0, 0 };
	};

	using SpriteSeriesMap = std::unordered_map<std::string_view, std::vector<SpriteInfo>>;
	SpriteSeriesMap spriteSeriesMap_;
};


//struct SpriteSeriesInfo
//{
//	std::string seriesName;
//	int currentIndex = -1;
//	std::vector<SDL_Rect> atlasRects;
//};


//auto getSeriesDimensions = 
//	[](const std::vector<std::pair<SpriteInfo, SDL_Surface*>>& spriteSurfaces) -> Dimensions<int> {
//		int totalWidth = std::accumulate(spriteSurfaces.begin(), spriteSurfaces.end(), 0,
//			[](int sum, const auto& pair) { return sum + pair.second->w; });

//	auto tallestSurface = std::max_element(spriteSurfaces.begin(), spriteSurfaces.end(),
//		[](const auto& lhs, const auto& rhs) { return lhs.second->h < rhs.second->h; });

//	return { totalWidth, tallestSurface->second->h };
//};

//Dimensions<int> unusedSpace = { 0, 0 };
//int maxSeriesWidth = 0, totalHeight = 0; 

//for (const auto& [_, spriteSurfaces] : loadingMap)
//{
//	auto [seriesWidth, seriesHeight] = getSeriesDimensions(spriteSurfaces);

//	maxSeriesWidth = std::max(seriesWidth, maxSeriesWidth);

//	unusedSpace

//	//if (seriesWidth < maxSeriesWidth)
//	//{
//	//	unusedSpace.w += maxSeriesWidth - seriesWidth;
//	//	unusedSpace.h += seriesHeight;
//	//}

//	totalHeight += seriesHeight;
//}

//int totalArea = maxSeriesWidth * totalHeight - (unusedSpace.w * unusedSpace.h);
//int minSide = static_cast<int>(std::ceil(std::sqrt(totalArea)));

//int atlasSideLen = GetNextPowerOfTwo(minSide);

//namespace fs = std::filesystem;
//
//for (const auto& series : atlasInfo_.seriesData)
//{
//	auto& surfaces = surfaceMap[dirPath];
//
//	try
//	{
//		for (const auto& entry : fs::directory_iterator(dirPath))
//		{
//			if (entry.is_regular_file())
//			{
//				const std::string fullPath = entry.path().string();
//
//				if (!surfaces.emplace_back(IMG_Load(fullPath.c_str())))
//				{
//					std::cerr << "Failed to load sprite from file : " << fullPath;
//
//					return false;
//				}
//			}
//		}
//	}
//	catch (const fs::filesystem_error& e)
//	{
//		std::cerr << "Filesystem error: " << e.what();
//
//		return false;
//	}
//	catch (const std::exception& e)
//	{
//		std::cerr << "General error: " << e.what();
//
//		return false;
//	}
//
//	return true;
//}

//int y = 0, x = 0, rowHeight = 0;
//
//for (auto& [seriesName, spriteSurfaces] : loadingMap)
//{
//	for (auto& [spriteInfo, surface] : spriteSurfaces)
//	{
//		if (x + surface->w > atlasSurface->w)
//		{
//			x = 0;
//			y += rowHeight;
//			rowHeight = 0;
//		}
//
//		assert(y + surface->h < atlasSideLen);
//
//		spriteInfo.atlasRect.x = x;
//		spriteInfo.atlasRect.y = y;
//
//		if (SDL_BlitSurface(surface, nullptr, atlasSurface, &spriteInfo.atlasRect) == -1)
//		{
//			std::cerr << "Failed to blit surface : " << SDL_GetError();
//
//			freeSurfaces();
//
//			return false;
//		}
//
//		x += surface->w;
//		rowHeight = std::max(rowHeight, surface->h);
//
//		spriteSeriesMap_[seriesName].emplace_back(std::move(spriteInfo));
//
//		SDL_FreeSurface(surface);
//	}
//}