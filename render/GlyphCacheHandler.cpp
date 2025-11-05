#include "GlyphCacheHandler.h"
#include "../core/commonObjects.h"
#include "GlyphFormattingUtils.h"

using namespace util;

namespace {

constexpr SDL_FPoint GetRectCenter(SDL_Rect rect)
{
	return { static_cast<float>(rect.x) + (static_cast<float>(rect.w) / 2.0f),
			 static_cast<float>(rect.y) + (static_cast<float>(rect.h) / 2.0f) };
}

constexpr SDL_Rect MakeProjectedBoundingRect(Dimensions<int> dims, SDL_FPoint scale)
{
	return SDL_Rect{ 0, 0,
		static_cast<int>(dims.w * scale.x),
		static_cast<int>(dims.h * scale.y)
	};
}

void FillGlyphRectsLeftAlign(std::vector<GlyphCacheData>& cache,
							 const FormatArgs& format)
{
	auto [xPos, yPos] = format.start;

	for (auto& [glyph, destRect, _] : cache)
	{
		assert(glyph.character != kInvalidChar);

		if (glyph == NewGlyphAtlas::kNewlineGlyph)
		{
			xPos = format.start.x;
			yPos += static_cast<int>(format.fontHeight * format.scale.y);

			continue;
		}

		destRect = { xPos, yPos, static_cast<int>(glyph.plot.rect.w * format.scale.x),
								 static_cast<int>(glyph.plot.rect.h * format.scale.y) };

		xPos += static_cast<int>(glyph.advance * format.scale.x);

	}
}

void FillGlyphRectsRightAlign(std::vector<GlyphCacheData>& cache,
							  const FormatArgs& format)
{
	auto [xPos, yPos] = format.start;

	yPos += static_cast<int>(format.fontHeight * format.scale.y) * format.numNewlines;

	for (int i = static_cast<int>(cache.size()) - 1; i >= 0; i--)
	{
		auto& [glyph, destRect, _] = cache[i];

		assert(glyph.character != kInvalidChar);

		if (glyph.character == '\n')
		{
			xPos = format.start.x;
			yPos -= static_cast<int>(format.fontHeight * format.scale.y);

			continue;
		}

		destRect = { xPos - static_cast<int>(glyph.plot.rect.w * format.scale.x), yPos,
					 static_cast<int>(glyph.plot.rect.w * format.scale.x),
					 static_cast<int>(glyph.plot.rect.h * format.scale.y) };

		xPos -= static_cast<int>(glyph.advance * format.scale.x);
	}
}

void FillGlyphRectsCenterAlign(std::string_view text, std::vector<GlyphCacheData>& cache,
							   const FormatArgs& format)
{
	assert(text.size() == cache.size());
	assert(format.numNewlines >= 0);

	int yPos = format.start.y;
	int currentTextPos = 0;

	for (size_t i = 0; i <= static_cast<size_t>(format.numNewlines); i++)
	{
		size_t newlinePos = text.find_first_of('\n', currentTextPos);
		newlinePos = std::min(newlinePos, text.size());

		int rowWidth = CalculateGlyphRowWidth(cache, currentTextPos,
			newlinePos, format.scale.x);

		int xPos = format.start.x - static_cast<int>(rowWidth / 2.0f);

		for (size_t j = currentTextPos; j < newlinePos; j++)
		{
			auto& [glyph, destRect, _] = cache[j];

			assert(glyph.character != kInvalidChar);

			destRect = { xPos , yPos, static_cast<int>(glyph.plot.rect.w * format.scale.x),
									  static_cast<int>(glyph.plot.rect.h * format.scale.y) };

			xPos += static_cast<int>(glyph.advance * format.scale.x);
		}

		yPos += static_cast<int>(format.fontHeight * format.scale.y);
		currentTextPos = newlinePos + 1;
	}
} 

} // unnamed

void GlyphCacheHandler::RepopulateGlyphCacheGlyphs(std::string_view text,
	std::vector<GlyphCacheData>& cache, const NewGlyphAtlas& glyphAtlas)
{
	cache.resize(text.size());

	for (size_t i = 0; i < text.size(); i++)
	{
		cache[i].destRect = { 0, 0, 0, 0 };
		cache[i].glyph = glyphAtlas.GetGlyph(text[i]);
	}
}

void GlyphCacheHandler::ReprojectGlyphCacheGeometry(
	std::vector<GlyphCacheData>& cache, const TextRenderableComponent& textRenderable,
	const Transform& transform, SDL_Rect projectedRect, const NewGlyphAtlas& glyphAtlas)
{
	auto format = MakeFormatArgs(textRenderable.writer.text, cache,
								 textRenderable.formatting.bounds,
								 textRenderable.formatting.scaleToBounds,
								 transform, glyphAtlas);

	switch (textRenderable.formatting.align)
	{
	case TextAlign::Left:
		format.start.x = projectedRect.x;
		FillGlyphRectsLeftAlign(cache, format);
		break;
	case TextAlign::Right:
		format.start.x = projectedRect.x + projectedRect.w;
		FillGlyphRectsRightAlign(cache, format);
		break;
	case TextAlign::Center: default:
		format.start.x = projectedRect.x + (projectedRect.w / 2);
		FillGlyphRectsCenterAlign(textRenderable.writer.text, cache, format);
		break;
	}
}

void GlyphCacheHandler::AdjustGlyphCacheRotation(std::vector<GlyphCacheData>& cache,
												 SDL_Rect projectedRenderRect,
												 float angleDegrees)
{
	SDL_FPoint bboxCenter = GetRectCenter(projectedRenderRect);

	float radians = angleDegrees * static_cast<float>(M_PI) / 180.0f;
	float cosA = std::cos(radians);
	float sinA = std::sin(radians);

	for (auto& [glyph, destRect, rotCenter] : cache)
	{
		SDL_Rect srcRect = glyph.plot.rect;

		SDL_FPoint destCenter = GetRectCenter(destRect);

		// Offset from bounding box center
		float dx = destCenter.x - bboxCenter.x;
		float dy = destCenter.y - bboxCenter.y;

		// Rotate position
		float rotatedX = dx * cosA - dy * sinA;
		float rotatedY = dx * sinA + dy * cosA;

		SDL_FPoint newPos = {
			bboxCenter.x + rotatedX,
			bboxCenter.y + rotatedY
		};

		// Adjust dest rect for new center
		destRect.x = static_cast<int>(newPos.x - destRect.w / 2.0f);
		destRect.y = static_cast<int>(newPos.y - destRect.h / 2.0f);

		rotCenter = {
			static_cast<int>(destRect.w / 2.0f),
			static_cast<int>(destRect.h / 2.0f)
		};
	}
}

void GlyphCacheHandler::AdjustGlyphCachePosition(TextRenderableGlyphCache& cacheComponent,
											     SDL_FPoint newPos,
											     SDL_FPoint newOffset)
{
	SDL_FPoint adjust = (newPos - cacheComponent.context.transform.position) +
						(newOffset - cacheComponent.context.offset);

	for (auto& cacheData : cacheComponent.cache)
	{
		cacheData.destRect.x += static_cast<int>(adjust.x);
		cacheData.destRect.y += static_cast<int>(adjust.y);
	}
}

void GlyphCacheHandler::UpdateGlyphCache(const NewGlyphAtlas& glyphAtlas,
										 TextRenderableComponent& textRenderable,
										 TextRenderableGlyphCache& glyphCache,
										 const Transform& transform)
{
	auto& [glyphs, ctx] = glyphCache;

	uint8_t changeLog = MakeChangeLog(textRenderable, transform, glyphCache);
	if (changeLog == NoChange)
	{
		return;
	}

	if (changeLog & (AtlasChanged | TextChanged))
	{
		RepopulateGlyphCacheGlyphs(textRenderable.writer.text, glyphCache.cache,
								   glyphAtlas);
	}

	if (changeLog & NeedsReprojection || changeLog & RotationChanged)
	{
		SDL_Rect projectedRect = MakeProjectedBoundingRect(
			textRenderable.formatting.bounds, transform.scale
		);

		if (changeLog & NeedsReprojection)
		{
			ReprojectGlyphCacheGeometry(glyphs, textRenderable, transform,
										projectedRect, glyphAtlas);
		}
		if (changeLog & RotationChanged)
		{
			AdjustGlyphCacheRotation(glyphs, projectedRect, transform.rotation);
		}
	}

	if (changeLog & PositionChanged)
	{
		AdjustGlyphCachePosition(glyphCache, transform.position,
								 textRenderable.profile.offset);
	}

	ctx.transform = transform;
	ctx.formatting = textRenderable.formatting;
	ctx.offset = textRenderable.profile.offset;
	ctx.sourceAtlas = textRenderable.writer.sourceAtlas;
}