#include "GlyphFormattingUtils.h"
#include "../../atlas/NewGlyphAtlas.h"
#include <numeric>
#include <cstdint>

namespace util {

namespace {

template <typename T>
constexpr T BitIf(bool condition, T flag) noexcept
{
	return static_cast<T>(-static_cast<int>(condition) & static_cast<int>(flag));
}

} // unnamed

bool operator==(std::string_view text, const std::vector<GlyphCacheData>& cacheData)
{
	if (text.size() != cacheData.size())
	{
		return false;
	}

	for (size_t i = 0; i < text.size(); i++)
	{
		if (text[i] != cacheData[i].glyph.character)
		{
			return false;
		}
	}

	return true;
}


uint8_t MakeChangeLog(const NewRenderable& renderable,
				      const NewTextRenderable& textRenderable,
					  const Transform& transform,
					  const TextRenderableGlyphCache& glyphCache)
{
	const auto& [glyphs, ctx] = glyphCache;

	return BitIf(textRenderable.writer.sourceAtlas != renderable.internals_.sourceAtlas, 
				 AtlasChanged) |
		   BitIf(textRenderable.writer.text != glyphs, 
			     TextChanged) |
		   BitIf(textRenderable.formatting != ctx.format, 
			     FormatChanged) |
		   BitIf(transform.rotation != ctx.transform.rotation, 
			     RotationChanged) |
		   BitIf(transform.position != ctx.transform.position ||
		   	     renderable.profile.offset != ctx.offset, 
			     PositionChanged) |
		   BitIf(transform.scale != ctx.transform.scale, 
			     ScaleChanged);
}

int CalculateGlyphRowWidth(const std::vector<GlyphCacheData>& cache,
						   int currentPos, int newlinePos, float scaleX)
{
	return std::accumulate(
		cache.begin() + currentPos,
		cache.begin() + newlinePos,
		0, [scale = scaleX](int sum, const auto& data) {
			return sum + static_cast<int>(data.glyph.advance * scale);
		});
}

FormatArgs MakeFormatArgs(std::string_view text, std::vector<GlyphCacheData>& cache,
						  Dimensions<int> bounds, bool scaleToFit,
						  const Transform& transform, const NewGlyphAtlas& glyphAtlas)
{
	int numNewlines = std::count(text.begin(), text.end(), '\n');

	int fontHeight = glyphAtlas.GetFontDescriptor().fontHeight;

	int totalHeight = static_cast<int>(fontHeight * transform.scale.y) *
		static_cast<int>(numNewlines + 1);

	int startY = static_cast<int>(transform.position.y - (totalHeight / 2.0f));

	FormatArgs format{
		.bounds = bounds,
		.scale = transform.scale,
		.start = { 0, startY }, // CHECK THIS
		.numNewlines = numNewlines,
		.fontHeight = fontHeight,
		.totalHeight = totalHeight
	};

	if (scaleToFit)
	{
		format.scale *= GetScaleToFitFactor(text, cache, format);
	}

	return format;
}

float GetScaleToFitFactor(std::string_view text,
						  std::vector<GlyphCacheData>& cache,
						  const FormatArgs& format)
{
	int currentPos = 0;
	int longestRowWidth = 0;
	for (int i = 0; i <= format.numNewlines; i++)
	{
		size_t newlinePos = text.find_first_of('\n', currentPos);
		newlinePos = std::min(newlinePos, text.size());

		int rowWidth = CalculateGlyphRowWidth(cache, currentPos,
			newlinePos, format.scale.x);

		longestRowWidth = std::max(longestRowWidth, rowWidth);

		currentPos = newlinePos + 1;
	}

	return std::min(format.bounds.w / static_cast<float>(longestRowWidth),
					format.bounds.h / static_cast<float>(format.totalHeight));
}

SDL_Rect ComputeGlyphDataBoundingBox(const std::vector<GlyphCacheData>& cache)
{
	if (cache.empty())
	{
		return { 0, 0, 0, 0 };
	}

	int minX = std::numeric_limits<int>::max();
	int minY = std::numeric_limits<int>::max();
	int maxX = std::numeric_limits<int>::lowest();
	int maxY = std::numeric_limits<int>::lowest();

	for (const auto& data : cache)
	{
		minX = std::min(minX, data.destRect.x);
		minY = std::min(minY, data.destRect.y);
		maxX = std::max(maxX, data.destRect.x + data.destRect.w);
		maxY = std::max(maxY, data.destRect.y + data.destRect.h);
	}

	return SDL_Rect{
		minX,
		minY,
		maxX - minX,
		maxY - minY
	};
}

} // util