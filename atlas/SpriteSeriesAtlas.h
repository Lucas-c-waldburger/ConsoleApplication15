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


struct SpriteAnimationSeries;

class SpriteSeriesAtlas : public Atlas<SpriteSeriesAtlas>
{
public:
	SpriteSeriesAtlas() = default;
	SpriteSeriesAtlas(const Handle<SpriteSeriesAtlas>& handle) : Atlas(handle) {}

	bool Load(SDL_Renderer* renderer, AtlasInfo args);
	SpriteInfo GetSprite(std::string_view seriesName, size_t index, bool wrapOnOutOfRange = true) const;

	SpriteAnimationSeries GetSpriteAnimationSeries(std::string_view seriesName) const;

private:
	struct AtlasSizeAccumulator
	{
		int totalArea = 0;
		Dimensions<int> maxDims = { 0, 0 };
	};

	// TODO: change map key from string_view to string. dont get cute
	using SpriteSeriesMap = std::unordered_map<std::string_view, std::vector<SpriteInfo>>;
	SpriteSeriesMap spriteSeriesMap_;
};


struct SpriteSeriesMetadata
{
	std::string seriesName;
};

using SpriteSeriesResourcePackets = ResourcePackets<SpriteSeriesMetadata>;

class SpriteAtlas : public TextureAtlas<SpriteAtlas>
{
public:
	friend class TextureAtlas<SpriteAtlas>;

	std::vector<AtlasPlot> GetSpritePlots(std::string_view seriesName) const;
	std::vector<std::string_view> GetSeriesNames() const;

private:
	Result<Void> LoadImpl(SDL_Renderer* renderer, SpriteSeriesResourcePackets&& packets);

private:
	SpriteSeriesResourcePackets resourcePackets_;
	std::unordered_map<std::string_view, std::vector<AtlasPlot>> plotsBySeries_;
};