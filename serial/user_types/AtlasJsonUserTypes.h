#pragma once
#include "../../atlas/NewTextureRepository.h"
#include "CoreJsonUserTypes.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AtlasPlot, rect, rotation)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Sprite, sourceAtlas)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpriteInfo, plot, spriteName, filepath,
	seriesName, seriesIndex)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpriteDescriptor, spriteName, filepath)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpriteDescriptors, data, seriesName)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FontDescriptor, fontName, filepath, 
	fontSize, fontHeight)


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

inline constexpr std::string_view kNoSeriesKey = "noSeries";
inline constexpr std::string_view kHashKey = "hash";
inline constexpr std::string_view kSpriteDescriptorsKey = "spriteDescriptors";
inline constexpr std::string_view kTextureSizeKey = "textureSize";
inline constexpr std::string_view kFontDescriptorKey = "fontDescriptor";

//inline constexpr std::string_view kBinPackKey = "binPack_";

//inline constexpr std::string_view kSpriteInfoKey = "spriteInfo_";
//inline constexpr std::string_view kSpriteIndicesKey = "spriteIndices_";
//inline constexpr std::string_view kSeriesRangesKey = "seriesRanges_";

/* SPRITE ATLAS */
inline void to_json(nlohmann::json& j, const SpriteAtlas& atlas)
{
	if (!atlas.IsLoaded())   
	{
		return;
	}

	j = {
		{ kHashKey, atlas.GetHandle().GetHash() },
		{ kTextureSizeKey, atlas.GetTextureSize() },
		{ kSpriteDescriptorsKey, atlas.ExportSpriteDescriptors() }
	};
}

/* GLYPH ATLAS */
inline void to_json(nlohmann::json& j, const GlyphAtlas& atlas)
{
	if (!atlas.IsLoaded())
	{
		return;
	}

	j = {
		{ kHashKey, atlas.GetHandle().GetHash() },
		{ kTextureSizeKey, atlas.GetTextureSize() },
		{ kFontDescriptorKey, atlas.GetFontDescriptor() }
	};
}

//inline void from_json(const nlohmann::json& j, SpriteAtlas& atlas)
//{
//	j.at(kTextureSizeKey).get_to(atlas.textureSize_);
//	j.at(kBinPackKey).get_to(atlas.binPack_);
//	j.at(kSpriteInfoKey).get_to(atlas.spriteInfo_);
//	j.at(kSpriteIndicesKey).get_to(atlas.spriteIndices_); 
//	j.at(kSeriesRangesKey).get_to(atlas.seriesRanges_);	
//}

	//j = {
	//	{ kTextureSizeKey, atlas.GetTextureSize() },
	//	{ kBinPackKey, atlas.binPack_ },
	//	{ kSpriteInfoKey, atlas.spriteInfo_ },
	//	{ kSpriteIndicesKey, atlas.spriteIndices_ },
	//	{ kSeriesRangesKey, atlas.seriesRanges_ }
	//};