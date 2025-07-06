#include "SpriteSeriesAtlas.h"
#include "../sprite/SpriteAnimationSeries.h"
#include "../core/Algorithms.h"
#include "../core/Result.h"
#include "PackingTools.h"
#include <numeric>
#include <algorithm>
#include <set>
#include <string>
#include <SDL_image.h>

namespace {

struct SpriteSurface
{
	std::string_view seriesName;
	AtlasPlot plot = { 0, 0, 0, 0 };
	SDL_Surface* surface = nullptr;
};

} // unnamed

std::vector<AtlasPlot> SpriteSeriesAtlas::GetSpritePlots(std::string_view seriesName) const
{
	if (!IsLoaded())
	{
		return {};
	}

	auto it = plotsBySeries_.find(seriesName);

	return (it != plotsBySeries_.end()) ? it->second : std::vector<AtlasPlot>{};
}

std::vector<std::string_view> SpriteSeriesAtlas::GetSeriesNames() const
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

Result<Void> SpriteSeriesAtlas::LoadImpl(SDL_Renderer* renderer, SpriteSeriesResourcePackets&& packets)
{
	plotsBySeries_.clear();

	if (packets.empty())
	{
		return MAKE_ERROR("Sprite series resource packets was empty");
	}

	resourcePackets_ = std::move(packets);

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
	size_t surfacesIdx = 0;
	for (const auto& packet : resourcePackets_)
	{
		if (!packet.IsValid())
		{
			return MAKE_ERROR("Sprite series resource packet was invalid");
		}

		const auto& metadata = packet.GetMetadata();
		const auto& filepaths = packet.GetFilepaths();

		for (const auto& filepath : filepaths)
		{
			auto& [seriesName, _, surface] = spriteSurfaces[surfacesIdx++];

			seriesName = metadata.seriesName;

			surface = IMG_Load(filepath.c_str());
			if (!surface)
			{
				freeResources();

				return MAKE_ERROR_FMT("Error loading surface: {}", SDL_GetError());
			}

			if (surface->w <= 0 || surface->h <= 0)
			{
				freeResources();

				return MAKE_ERROR_FMT("Surface dimensions invalid: [{}, {}]", surface->w, surface->h);
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

	bool done = false;
	while (!done)
	{
		done = true;

		if (atlasSideLen > (1 << 30))
		{
			freeResources();

			return MAKE_ERROR("Not all rects could be packed in the maximum atlas size");
		}

		binPack_.Init(atlasSideLen, atlasSideLen, false);

		for (auto& [seriesName, plot, surface] : spriteSurfaces)
		{
			plot.rotation = 0.0f; // undo any rotation from a prev pass

			rbp::Rect packed = binPack_.Insert(surface->w, surface->h,
											   rbp::MaxRectsBinPack::RectBestAreaFit);
			if (!WasRectPacked(packed))
			{
				atlasSideLen *= 2;

				done = false;

				break;
			}

			plot.rect = RbpToSDLRect(packed);

			bool wasFlipped = (plot.rect.w == surface->h && plot.rect.h == surface->w);
			if (wasFlipped)
			{
				plot.rotation = 90.0f;
			}
		}
	}

	atlasSurface = SDL_CreateRGBSurfaceWithFormat(
		0, atlasSideLen, atlasSideLen, 32, SDL_PIXELFORMAT_RGBA32);

	for (auto& [seriesName, plot, surface] : spriteSurfaces)
	{
		int blitted = SDL_BlitSurface(surface, nullptr, atlasSurface, &plot.rect);
		if (blitted < 0)
		{
			freeResources();

			return MAKE_ERROR_FMT("Failed to blit surface: {}", SDL_GetError());
		}

		SDL_FreeSurface(surface);

		auto it = plotsBySeries_.find(seriesName);
		assert(it != plotsBySeries_.end());
		
		it->second.emplace_back(plot);
	}

	atlasTexture_ = MakeUniqueTexturePtrFromSurface(renderer, atlasSurface);

	SDL_FreeSurface(atlasSurface);

	if (!atlasTexture_)
	{
		plotsBySeries_.clear();

		return MAKE_ERROR_FMT("Failed to create atlas texture: {}", SDL_GetError());
	}

	SDL_SetTextureBlendMode(atlasTexture_.get(), SDL_BLENDMODE_BLEND);

	return Void{};
}
