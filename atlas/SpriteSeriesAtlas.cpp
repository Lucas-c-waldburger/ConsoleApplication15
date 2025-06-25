#include "SpriteSeriesAtlas.h"
#include "SkylinePacker.h"
#include "../sprite/SpriteAnimationSeries.h"
#include "../core/Algorithms.h"
#include "../core/Result.h"
#include "PackingTools.h"
#include <numeric>
#include <algorithm>
#include <set>
#include <string>

namespace {

struct SpriteSurface
{
	std::string_view seriesName;
	AtlasPlot plot = { 0, 0, 0, 0 };
	SDL_Surface* surface = nullptr;
};

} // unnamed

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

			// this is where index actually gets assigned to the SpriteInfos
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

			SDL_Rect target = spriteInfo.atlasRect;

			if (SDL_BlitSurface(surface, nullptr, atlasSurface, &target) == -1)
			{
				std::cerr << "Failed to blit surface : " << SDL_GetError();

				freeResources();

				return false;
			}

			// make sure we didn't try to render out of bounds of the atlas texture
			//assert(spriteInfo.atlasRect == target);

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

SpriteAnimationSeries SpriteSeriesAtlas::GetSpriteAnimationSeries(std::string_view seriesName) const
{
	auto it = spriteSeriesMap_.find(seriesName);
	if (it == spriteSeriesMap_.end()) 
	{ 
		return {}; 
	}

	assert(!it->second.empty());

	SpriteAnimationSeries animSeries{ 
		.sourceAtlas = GetHandle(),
		.index = 0,
	};

	animSeries.spritePlots.resize(it->second.size());

	std::transform(it->second.begin(), it->second.end(), animSeries.spritePlots.begin(),
		[](const auto& spriteInfo) { return spriteInfo.atlasRect; });

	return animSeries;
}

std::vector<AtlasPlot> SpriteAtlas::GetSpritePlots(std::string_view seriesName) const
{
	if (!IsLoaded())
	{
		return {};
	}

	auto it = plotsBySeries_.find(seriesName);

	return (it != plotsBySeries_.end()) ? it->second : std::vector<AtlasPlot>{};
}

std::vector<std::string_view> SpriteAtlas::GetSeriesNames() const
{
	if (!IsLoaded())
	{
		return {};
	}

	std::vector<std::string_view> seriesNames{ plotsBySeries_.size() };

	std::transform(plotsBySeries_.begin(), plotsBySeries_.end(), seriesNames.begin(),
		[](const auto& pair) { return std::string_view{ pair.first }; });

	return seriesNames;
}

Result<Void> SpriteAtlas::LoadImpl(SDL_Renderer* renderer, SpriteSeriesResourcePackets&& packets)
{
	plotsBySeries_.clear();

	if (packets.empty())
	{
		return MAKE_ERROR("Sprite series resource packets was empty");
	}

	int numSprites = std::accumulate(resourcePackets_.begin(), resourcePackets_.end(), 0,
		[](int sum, const auto& packet) { return sum + static_cast<int>(packet.GetFilepaths().size()); });

	std::vector<SpriteSurface> spriteSurfaces{static_cast<size_t>(numSprites)};
	SDL_Surface* atlasSurface = nullptr;

	auto freeResources = [this, &spriteSurfaces, &atlasSurface]() {
		for (auto& spriteSurface : spriteSurfaces)
		{
			if (spriteSurface.surface)
			{
				SDL_FreeSurface(spriteSurface.surface);
			}
		}
		if (atlasSurface)
		{
			SDL_FreeSurface(atlasSurface);
		}

		plotsBySeries_.clear();
	};

	plotsBySeries_.reserve(resourcePackets_.size());

	int totalArea = 0;
	for (const auto& packet : resourcePackets_)
	{
		if (!packet.IsValid())
		{
			return MAKE_ERROR("Sprite series resource packet was invalid");
		}

		const auto& metadata = packet.GetMetadata();
		const auto& filepaths = packet.GetFilepaths();

		for (size_t i = 0; i < filepaths.size(); i++)
		{
			auto& [seriesName, plot, surface] = spriteSurfaces[i];

			seriesName = metadata.seriesName;

			surface = IMG_Load(filepaths[i].c_str());
			if (!surface)
			{
				freeResources();

				return MAKE_ERROR_FMT("Error loading surface: {}", SDL_GetError());
			}

			plot.rect.w = surface->w;
			plot.rect.h = surface->h;

			if (plot.rect.w <= 0 || plot.rect.h <= 0)
			{
				freeResources();

				return MAKE_ERROR_FMT("Surface dimensions invalid: [{}, {}]", plot.rect.w, plot.rect.h);
			}

			totalArea += surface->w * surface->h;
		}

		auto [it, success] = plotsBySeries_.emplace(metadata.seriesName, std::vector<AtlasPlot>{});
		if (!success)
		{
			freeResources();

			return MAKE_ERROR_FMT("Repeated sprite series name: '{}'", metadata.seriesName);
		}

		it->second.reserve(filepaths.size());
	}

	float idealSideLen = std::ceil(std::sqrt(static_cast<float>(totalArea)));
	int atlasSideLen = GetNextPowerOfTwo(static_cast<int>(idealSideLen));

	bool success = false;
	while (!success)
	{
		if (atlasSideLen > (1 << 30))
		{
			freeResources();

			return MAKE_ERROR("Not all rects could be packed in the maximum atlas size");
		}

		binPack_.Init(atlasSideLen, atlasSideLen);

		for (auto& [seriesName, plot, surface] : spriteSurfaces)
		{
			rbp::Rect packed = binPack_.Insert(plot.rect.w, plot.rect.h,
											   rbp::MaxRectsBinPack::RectBestAreaFit);
			if (!WasRectPacked(packed))
			{
				atlasSideLen = GetNextPowerOfTwo(atlasSideLen);

				continue;
			}

			plot.rect.x = packed.x;
			plot.rect.y = packed.y;

			if (WasFlipped(packed, plot.rect))
			{
				plot.rotation = 90.0f;
			}

			plotsBySeries_[seriesName].emplace_back(plot);
		}

		success = true;
	}

	atlasSurface = SDL_CreateRGBSurfaceWithFormat(
		0, atlasSideLen, atlasSideLen, 32, SDL_PIXELFORMAT_RGBA32);

	for (auto& [_, plot, surface] : spriteSurfaces)
	{
		int blitted = SDL_BlitSurface(surface, nullptr, atlasSurface, &plot.rect);
		if (blitted < 0)
		{
			freeResources();

			return MAKE_ERROR_FMT("Failed to blit surface: {}", SDL_GetError());
		}

		SDL_FreeSurface(surface);
	}

	atlasTexture_ = SDL_CreateTextureFromSurface(renderer, atlasSurface);

	SDL_FreeSurface(atlasSurface);

	if (!atlasTexture_)
	{
		plotsBySeries_.clear();

		return MAKE_ERROR_FMT("Failed to create atlas texture: {}", SDL_GetError());
	}

	SDL_SetTextureBlendMode(atlasTexture_, SDL_BLENDMODE_BLEND);

	return Void{};
}
