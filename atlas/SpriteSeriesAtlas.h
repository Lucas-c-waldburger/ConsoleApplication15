#pragma once
#include "Atlas.h"
#include "../core/commonObjects.h"
#include "../core/TransparentStringHash.h"
#include <unordered_map>

struct SpriteSeriesMetadata
{
	std::string seriesName;
};

using SpriteSeriesResourcePackets = ResourcePackets<SpriteSeriesMetadata>;

class SpriteSeriesAtlas : public TextureAtlas<SpriteSeriesAtlas>
{
public:
	friend class TextureAtlas<SpriteSeriesAtlas>;

	SpriteSeriesAtlas() = default;
	~SpriteSeriesAtlas() = default;

	SpriteSeriesAtlas(const SpriteSeriesAtlas&) = delete;
	SpriteSeriesAtlas& operator=(const SpriteSeriesAtlas&) = delete;

	SpriteSeriesAtlas(SpriteSeriesAtlas&& other) noexcept : TextureAtlas<SpriteSeriesAtlas>(std::move(other)),
		resourcePackets_(std::move(other.resourcePackets_)), plotsBySeries_(std::move(other.plotsBySeries_)) {}
	SpriteSeriesAtlas& operator=(SpriteSeriesAtlas&& other) noexcept
	{
		if (this != &other)
		{
			TextureAtlas<SpriteSeriesAtlas>::operator=(std::move(other));
			resourcePackets_ = std::move(other.resourcePackets_);
			plotsBySeries_ = std::move(other.plotsBySeries_);
		}
		return *this;
	}

	std::vector<AtlasPlot> GetSpritePlots(std::string_view seriesName) const;
	std::vector<std::string_view> GetSeriesNames() const;

private:
	Result<Void> LoadImpl(SDL_Renderer* renderer, SpriteSeriesResourcePackets&& packets);

private:
	SpriteSeriesResourcePackets resourcePackets_;
	std::unordered_map<std::string, std::vector<AtlasPlot>, 
		TransparentStringHash, std::equal_to<>> plotsBySeries_;
};