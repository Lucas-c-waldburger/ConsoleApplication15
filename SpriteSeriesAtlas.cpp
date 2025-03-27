#include "SpriteSeriesAtlas.h"
#include "SkylinePacker.h"
#include <numeric>

bool SpriteSeriesAtlas::Load(SDL_Renderer* renderer, AtlasInfo args)
{
	assert(renderer);

	atlasInfo_ = std::move(args);

	using LoadingMap = std::unordered_map<std::string_view,
		std::vector<std::pair<SpriteInfo, SDL_Surface*>>>;
	LoadingMap loadingMap;
	loadingMap.reserve(atlasInfo_.seriesDatas.size());

	SDL_Surface* atlasSurface = nullptr;

	auto freeResources = [this, &loadingMap, &atlasSurface]() {
		for (auto& [_, spriteSurfaces] : loadingMap)
		{
			for (auto& [_, surface] : spriteSurfaces)
			{
				if (surface) { SDL_FreeSurface(surface); }
			}
		}
		if (atlasSurface) { SDL_FreeSurface(atlasSurface); }

		spriteSeriesMap_.clear();
	};

	AtlasSizeAccumulator sizeAccumulator{};

	// load our surfaces, accumulate size info as we do
	for (const auto& series : atlasInfo_.seriesDatas)
	{
		auto& spriteInfoSurfaces = loadingMap[series.seriesName];
		spriteInfoSurfaces.reserve(series.spriteFilepaths.size());

		for (int i = 0; i < series.spriteFilepaths.size(); i++)
		{
			const auto& filePath = series.spriteFilepaths[i];

			auto& [newSpriteInfo, newSurface] = spriteInfoSurfaces.emplace_back();

			newSurface = IMG_Load(filePath.c_str());
			if (!newSurface)
			{
				std::cerr << "Failed to load sprite from path : " << filePath;

				freeResources();

				return false;
			}

			newSpriteInfo.index = i;
			newSpriteInfo.atlasRect = { 0, 0, newSurface->w, newSurface->h };

			sizeAccumulator.totalArea += newSurface->w * newSurface->h;
			sizeAccumulator.maxDims.w = std::max(sizeAccumulator.maxDims.w, newSurface->w);
			sizeAccumulator.maxDims.h = std::max(sizeAccumulator.maxDims.h, newSurface->h);
		}
	}

	// surfaces all loaded, lets get atlas texture's side length from our accumulator
	int minSide = static_cast<int>(std::ceil(std::sqrt(sizeAccumulator.totalArea)));
	int atlasSideLen = GetNextPowerOfTwo(minSide);

	// make atlas and blit surfaces to it
	atlasSurface = SDL_CreateRGBSurfaceWithFormat(
		0, atlasSideLen, atlasSideLen, 32, SDL_PIXELFORMAT_RGBA32);

	SkylinePacker skylinePacker{ atlasSideLen, atlasSideLen };

	for (auto& [seriesName, spriteSurfaces] : loadingMap)
	{
		for (auto& [spriteInfo, surface] : spriteSurfaces)
		{
			auto placementPos = skylinePacker.Insert(surface->w, surface->h);

			assert(placementPos.has_value());

			spriteInfo.atlasRect.x = placementPos->x;
			spriteInfo.atlasRect.y = placementPos->y;

			if (SDL_BlitSurface(surface, nullptr, atlasSurface, &spriteInfo.atlasRect) == -1)
			{
				std::cerr << "Failed to blit surface : " << SDL_GetError();

				freeResources();

				return false;
			}


			spriteSeriesMap_[seriesName].emplace_back(std::move(spriteInfo));

			SDL_FreeSurface(surface);
		}
	}

	atlasTexture_ = SDL_CreateTextureFromSurface(renderer, atlasSurface);

	SDL_FreeSurface(atlasSurface);

	if (!atlasTexture_)
	{
		std::cerr << "Failed to create atlas texture : " << SDL_GetError();

		spriteSeriesMap_.clear();

		return false;
	}

	SDL_SetTextureBlendMode(atlasTexture_, SDL_BLENDMODE_BLEND);

	return true;
}

SpriteInfo SpriteSeriesAtlas::GetSprite(std::string_view seriesName, size_t index, bool wrapOnOutOfRange) const
{
	auto it = spriteSeriesMap_.find(seriesName);

	if (it == spriteSeriesMap_.end()) { return {}; }

	assert(!it->second.empty());

	if (it->second.size() >= index)
	{
		return (wrapOnOutOfRange) ? it->second[0] : SpriteInfo{};
	}

	return it->second[index];
}