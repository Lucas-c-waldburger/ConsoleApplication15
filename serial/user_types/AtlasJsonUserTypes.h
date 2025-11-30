#pragma once
#include "../../atlas/NewTextureRepository.h"
#include "CoreJsonUserTypes.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AtlasPlot, rect, rotation)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Sprite, sourceAtlas)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpriteInfo, plot, spriteName, filepath,
								   seriesName, seriesIndex)

/* SPRITE INFO SOA*/
inline constexpr std::string_view kPlotKey = "plot";
inline constexpr std::string_view kSpriteNameKey = "spriteName";
inline constexpr std::string_view kFilepathKey = "filepath";
inline constexpr std::string_view kSeriesName = "seriesName";
inline constexpr std::string_view kSeriesIndex = "seriesIndex";

template <typename BasicJson>
inline void to_json(BasicJson& j, const SpriteInfoSOA& spriteInfoSoa)
{
	auto& obj = BasicJson::object();

	auto& plots  = obj[kPlotKey]	   = BasicJson::array();
	auto& names  = obj[kSpriteNameKey] = BasicJson::array();
	auto& paths  = obj[kFilepathKey]   = BasicJson::array();
	auto& series = obj[kSeriesName]    = BasicJson::array();
	auto& idxs   = obj[kSeriesIndex]   = BasicJson::array();

	for (const auto& [plot, name, path, series, idx] : spriteInfoSoa.ForEach())
	{
		plots.push_back(plot);
		names.push_back(name);
		paths.push_back(path);
		series.push_back(series);
		idxs.push_back(idx);
	}
}

template <typename BasicJson>
inline void from_json(const BasicJson& j, SpriteInfoSOA& spriteInfoSoa)
{
	if (!j.is_object())
	{
		return;
	}

	const auto& plots  = j[kPlotKey];
	const auto& names  = j[kSpriteNameKey];
	const auto& paths  = j[kFilepathKey];
	const auto& series = j[kSeriesName];
	const auto& idxs   = j[kSeriesIndex];

	for (size_t i = 0; i < plots.size(); i++)
	{
		spriteInfoSoa.PushBack({
			.plot = plots[i],
			.spriteName = names[i],
			.filepath = paths[i],
			.seriesName = series[i],
			.seriesIndex = idxs[i]
		});
	}
}

inline constexpr std::string_view kSeriesIndexRangesKey = "seriesIndexRanges";
inline constexpr std::string_view kSpriteInfoKey = "spriteInfo";
inline constexpr std::string_view kTextureSizeKey = "textureSize";

/* SPRITE ATLASES */
template <typename BasicJson>
inline void to_json(BasicJson& j, const BaseSpriteAtlas& atlas)
{
	auto& obj = BasicJson::object();

	obj[kSeriesIndexRangesKey] = atlas.spriteSeriesRanges_;
	obj[kSpriteInfoKey] = atlas.spriteInfo_;
	obj[kTextureSizeKey] = atlas.GetTextureSize();
}

//template <typename BasicJson>
//inline void from_json(const nlohmann::json& j, FixedSpriteAtlas& atlas)
//{
//	j.at(kSeriesIndexRanges)get_to(atlas.)
//}