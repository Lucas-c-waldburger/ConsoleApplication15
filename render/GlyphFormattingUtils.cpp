#include "GlyphFormattingUtils.h"
#include "../atlas/NewGlyphAtlas.h"
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


uint8_t MakeChangeLog(const TextRenderableComponent& textRenderable,
					  const Transform& transform,
					  const TextRenderableGlyphCache& glyphCache)
{
	const auto& ctx = glyphCache.context;

	return BitIf(textRenderable.writer.resourceHandle != ctx.resourceHandle, 
				 AtlasChanged) |
		   BitIf(RapidHash(textRenderable.writer.text) != ctx.textHash, 
			     TextChanged) |
		   BitIf(textRenderable.formatting != ctx.formatting, 
			     FormatChanged) |
		   BitIf(transform.rotation != ctx.transform.rotation, 
			     RotationChanged) |
		   BitIf(transform.position != ctx.transform.position ||
		   	     textRenderable.profile.offset != ctx.offset, 
			     PositionChanged) |
		   BitIf(transform.scale != ctx.transform.scale ||
			     textRenderable.formatting.scaleToBounds != ctx.formatting.scaleToBounds, 
			     ScaleChanged);
}

float CalculateGlyphRowWidth(const std::vector<GlyphCacheData>& cache,
							 size_t currentPos, size_t newlinePos, float scaleX)
{
	return std::accumulate(
		cache.begin() + currentPos,
		cache.begin() + newlinePos,
		0.0f, [scale = scaleX](float sum, const auto& data) {
			return sum + static_cast<float>(data.glyph.advance) * scale;
		});
}

float CalculateLongestRowWidth(std::string_view text, const FormatArgs& format,
						       const std::vector<GlyphCacheData>& cache)
{
	// Compute the longest row width (in layout-space) so we can anchor center/right if needed
	size_t currentPos = 0;
	float longestRowWidth = 0;
	for (int i = 0; i <= format.numNewlines; ++i)
	{
		size_t newlinePos = text.find_first_of('\n', currentPos);
		newlinePos = std::min(newlinePos, text.size());

		float rowWidth = CalculateGlyphRowWidth(cache, currentPos, newlinePos,
											    format.layoutScale.x);

		longestRowWidth = std::max(longestRowWidth, rowWidth);

		currentPos = newlinePos + 1;
	}

	return longestRowWidth;
}

float GetFormatArgsStartX(const TextRenderableComponent& textRenderable, const FormatArgs& formatArgs, 
						  const std::vector<GlyphCacheData>& cache)
{	
	if (textRenderable.formatting.align == TextAlign::Left)
	{
		return 0.0f;
	}
	
	float startX = static_cast<float>(formatArgs.bounds.w) - 
		CalculateLongestRowWidth(textRenderable.writer.text, formatArgs, cache);

	if (textRenderable.formatting.align == TextAlign::Center)
	{
		startX /= 2.0f;
	}

	return startX;
}

FormatArgs MakeFormatArgs(const TextRenderableComponent& textRenderable, 
						  const TextRenderableGlyphCache& cacheComponent, int fontHeight)
{
	int numNewlines = std::count(textRenderable.writer.text.begin(), 
								 textRenderable.writer.text.end(), '\n');

	FormatArgs formatArgs{
		.bounds = textRenderable.formatting.bounds,
		.numNewlines = numNewlines,
		.fontHeight = fontHeight,
		.totalHeight = static_cast<float>(fontHeight * (numNewlines + 1))
	};

	if (textRenderable.formatting.scaleToBounds)
	{
		float layoutScaleFactor = GetScaleToFitFactor(textRenderable.writer.text, 
													  cacheComponent.cache, formatArgs);

		formatArgs.layoutScale = { layoutScaleFactor, layoutScaleFactor };

		formatArgs.totalHeight = 
			formatArgs.fontHeight * formatArgs.layoutScale.y *
			static_cast<float>(numNewlines + 1);
	}

	formatArgs.start.x = GetFormatArgsStartX(textRenderable, formatArgs, cacheComponent.cache);

	return formatArgs;
}

float GetScaleToFitFactor(std::string_view text,
						  const std::vector<GlyphCacheData>& cache,
						  const FormatArgs& formatArgs)
{
	size_t currentPos = 0;
	float longestRowWidth = 0;
	for (int i = 0; i <= formatArgs.numNewlines; i++)
	{
		size_t newlinePos = text.find_first_of('\n', currentPos);
		newlinePos = std::min(newlinePos, text.size());

		float rowWidth = CalculateGlyphRowWidth(cache, currentPos,
			newlinePos, formatArgs.layoutScale.x);

		longestRowWidth = std::max(longestRowWidth, rowWidth);

		currentPos = newlinePos + 1;
	}

	return std::min(formatArgs.bounds.w / longestRowWidth,
					formatArgs.bounds.h / formatArgs.totalHeight);
}

SDL_FRect ComputeGlyphDataBoundingBox(const std::vector<GlyphCacheData>& cache)
{
	if (cache.empty())
	{
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	float minX = std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float maxY = std::numeric_limits<float>::lowest();

	for (const auto& data : cache)
	{
		minX = std::min(minX, data.destRect.x);
		minY = std::min(minY, data.destRect.y);
		maxX = std::max(maxX, data.destRect.x + data.destRect.w);
		maxY = std::max(maxY, data.destRect.y + data.destRect.h);
	}

	return SDL_FRect{
		minX,
		minY,
		maxX - minX,
		maxY - minY
	};
}

} // util