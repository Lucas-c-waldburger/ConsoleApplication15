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
	const auto& [glyphs, ctx] = glyphCache;

	return BitIf(textRenderable.writer.sourceAtlas != ctx.sourceAtlas, 
				 AtlasChanged) |
		   BitIf(textRenderable.writer.text != glyphs, 
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

int CalculateLongestRowWidth(std::string_view text, const FormatArgs& format,
						     const std::vector<GlyphCacheData>& cache)
{
	// Compute the longest row width (in layout-space) so we can anchor center/right if needed
	int currentPos = 0;
	int longestRowWidth = 0;
	for (int i = 0; i <= format.numNewlines; ++i)
	{
		size_t newlinePos = text.find_first_of('\n', currentPos);
		newlinePos = std::min(newlinePos, text.size());

		int rowWidth = CalculateGlyphRowWidth(cache, currentPos, static_cast<int>(newlinePos),
											  format.layoutScale.x);

		longestRowWidth = std::max(longestRowWidth, rowWidth);

		currentPos = static_cast<int>(newlinePos) + 1;
	}

	return longestRowWidth;
}

int GetFormatArgsStartX(const TextRenderableComponent& textRenderable, const FormatArgs& formatArgs, 
						const std::vector<GlyphCacheData>& cache)
{	
	if (textRenderable.formatting.align == TextAlign::Left)
	{
		return 0;
	}
	
	int startX = formatArgs.bounds.w - CalculateLongestRowWidth(textRenderable.writer.text, 
																formatArgs, cache);

	if (textRenderable.formatting.align == TextAlign::Center)
	{
		startX /= 2;
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
		.totalHeight = fontHeight * (numNewlines + 1)
	};

	if (textRenderable.formatting.scaleToBounds)
	{
		float layoutScaleFactor = GetScaleToFitFactor(textRenderable.writer.text, 
													  cacheComponent.cache, formatArgs);

		formatArgs.layoutScale = { layoutScaleFactor, layoutScaleFactor };

		formatArgs.totalHeight = 
			static_cast<int>(formatArgs.fontHeight * formatArgs.layoutScale.y) *
			(numNewlines + 1);
	}

	formatArgs.start.x = GetFormatArgsStartX(textRenderable, formatArgs, cacheComponent.cache);

	return formatArgs;
}

float GetScaleToFitFactor(std::string_view text,
						  const std::vector<GlyphCacheData>& cache,
						  const FormatArgs& formatArgs)
{
	int currentPos = 0;
	int longestRowWidth = 0;
	for (int i = 0; i <= formatArgs.numNewlines; i++)
	{
		size_t newlinePos = text.find_first_of('\n', currentPos);
		newlinePos = std::min(newlinePos, text.size());

		int rowWidth = CalculateGlyphRowWidth(cache, currentPos,
			newlinePos, formatArgs.layoutScale.x);

		longestRowWidth = std::max(longestRowWidth, rowWidth);

		currentPos = newlinePos + 1;
	}

	return std::min(formatArgs.bounds.w / static_cast<float>(longestRowWidth),
					formatArgs.bounds.h / static_cast<float>(formatArgs.totalHeight));
}

SDL_FRect ComputeGlyphDataBoundingBox(const std::vector<GlyphCacheData>& cache)
{
	if (cache.empty())
	{
		return { 0.0f, 0.0f, 0.0f, 0.0f };
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

	return SDL_FRect{
		static_cast<float>(minX),
		static_cast<float>(minY),
		static_cast<float>(maxX - minX),
		static_cast<float>(maxY - minY)
	};
}

} // util