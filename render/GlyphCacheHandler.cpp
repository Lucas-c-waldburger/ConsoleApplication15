#include "GlyphCacheHandler.h"
#include "../core/commonObjects.h"
#include "GlyphFormattingUtils.h"
#include "../systems/util/DebugDrawUtils.h"

//// TODO: Since we calculate rotation center in AddGlyphRenderCalls, do we even need
// to cache it? Furthermore, if its always just the center of the glyph, can it just be null?

using namespace util;

namespace {

template <SDLRectType R>
constexpr SDL_FPoint GetRectCenter(R rect)
{
	return { static_cast<float>(rect.x) + (static_cast<float>(rect.w) / 2.0f),
			 static_cast<float>(rect.y) + (static_cast<float>(rect.h) / 2.0f) };
}

constexpr SDL_FPoint GetClampedScale(const Transform& tf)
{
	return { std::max(tf.scale.x, 0.1f), std::max(tf.scale.y, 0.1f) };
}

struct TempGlyphRects
{
	std::vector<SDL_FRect> rects;
	SDL_FRect combinedBBox = { 0.0f, 0.0f, 0.0f };
};

void FillGlyphRects(std::vector<GlyphCacheData>& cache, int fontHeight, const TextFormatting& format)
{
	if (cache.empty())
	{
		return;
	}

	std::vector<SDL_FRect> destRects;
	destRects.reserve(cache.size());
	std::vector<size_t> rowIndices;

	float xPos = 0.0f;
	float yPos = 0.0f;

	for (size_t i = 0; i < cache.size(); ++i)
	{
		const auto& glyph = cache[i].glyph;
		assert(glyph.character != Glyph::kInvalidChar);

		if (glyph == FontAtlasTexture::kNewlineGlyph)
		{
			rowIndices.emplace_back(i);
			destRects.emplace_back(xPos, yPos, 0.0f, 0.0f);

			xPos = 0;
			yPos += static_cast<float>(fontHeight);

			continue;
		}

		destRects.emplace_back(
			xPos,
			yPos,
			static_cast<float>(glyph.plot.rect.w),
			static_cast<float>(glyph.plot.rect.h)
		);

		xPos += static_cast<float>(glyph.advance) * format.letterSpacing;
	}

	rowIndices.emplace_back(cache.size());
	destRects.emplace_back(xPos, yPos, 0.0f, 0.0f);

	auto combinedBBox = util::ComputeBoundingBox(destRects);

	const bool doScale = format.scaleToBounds && format.bounds.w > 0 && format.bounds.h > 0;
	if (doScale)
	{
		const float scale = std::min(
			static_cast<float>(format.bounds.w) / combinedBBox.w,
			static_cast<float>(format.bounds.h) / combinedBBox.h
		);

		for (auto& r : destRects)
		{
			r.x = (r.x - combinedBBox.x) * scale;
			r.y = (r.y - combinedBBox.y) * scale;
			r.w *= scale;
			r.h *= scale;
		}
	}

	if (format.align != TextAlign::Left)
	{
		const float boundsW = static_cast<float>(format.bounds.w);
		const float longestRowWidth = (doScale || boundsW > combinedBBox.w) ? boundsW : combinedBBox.w;

		size_t counter = 0;
		for (const auto rowIdx : rowIndices)
		{
			//if (counter >= destRects.size())
			//{
			//	break;
			//}

			const float rowWidth = destRects[rowIdx].x + destRects[rowIdx].w;

			/* (format.align == TextAlign::Right) */
			float xShift = longestRowWidth - rowWidth;
			if (format.align == TextAlign::Center)
			{
				xShift /= 2.0f;
			}

			while (counter <= rowIdx)
			{
				destRects[counter].x += xShift;
				++counter;
			}
		}
	}

	assert(cache.size() + 1 == destRects.size());

	for (size_t i = 0; i < destRects.size() - 1; ++i)
	{
		cache[i].destRect = destRects[i];
	}
}

//void ScaleRects(TempGlyphRects& temp, SDL_FPoint scale, const Dimensions<int>& bounds,
//				bool scaleToBounds)
//{
//
//}

void FillGlyphRectsLeftAlign(std::vector<GlyphCacheData>& cache,
							 const FormatArgs& format)
{
	auto [xPos, yPos] = format.start;

	for (auto& [glyph, destRect, _] : cache)
	{
		assert(glyph.character != Glyph::kInvalidChar);

		if (glyph == FontAtlasTexture::kNewlineGlyph)
		{
			xPos = format.start.x;
			yPos += static_cast<float>(format.fontHeight) * format.layoutScale.y;

			continue;
		}

		destRect = { 
			static_cast<float>(xPos), 
			static_cast<float>(yPos), 
			static_cast<float>(glyph.plot.rect.w * format.layoutScale.x),
			static_cast<float>(glyph.plot.rect.h * format.layoutScale.y) };

		xPos += static_cast<float>(glyph.advance) * format.layoutScale.x;
	}
}

void FillGlyphRectsRightAlign(std::vector<GlyphCacheData>& cache,
							  const FormatArgs& format)
{
	auto [xPos, yPos] = format.start;

	yPos += static_cast<float>(format.fontHeight) * format.layoutScale.y * 
			static_cast<float>(format.numNewlines);

	for (int i = static_cast<int>(cache.size()) - 1; i >= 0; i--)
	{
		auto& [glyph, destRect, _] = cache[i];

		assert(glyph.character != Glyph::kInvalidChar);

		if (glyph.character == '\n')
		{
			xPos = format.start.x;
			yPos -= static_cast<float>(format.fontHeight) * format.layoutScale.y;

			continue;
		}

		xPos -= static_cast<float>(glyph.advance) * format.layoutScale.x;

		destRect = { 
			static_cast<float>(xPos), 
			static_cast<float>(yPos), 
			static_cast<float>(glyph.plot.rect.w * format.layoutScale.x),
			static_cast<float>(glyph.plot.rect.h * format.layoutScale.y) 
		};
	}
}

void FillGlyphRectsCenterAlign(std::string_view text, std::vector<GlyphCacheData>& cache,
							   const FormatArgs& format)
{
	assert(text.size() == cache.size());
	assert(format.numNewlines >= 0);

	float yPos = format.start.y;
	size_t currentTextPos = 0;

	for (size_t i = 0; i <= static_cast<size_t>(format.numNewlines); i++)
	{
		size_t newlinePos = text.find_first_of('\n', currentTextPos);
		newlinePos = std::min(newlinePos, text.size());

		float rowWidth = CalculateGlyphRowWidth(cache, currentTextPos,
												newlinePos, format.layoutScale.x);

		float xPos = format.start.x - (rowWidth / 2.0f);

		for (size_t j = currentTextPos; j < newlinePos; j++)
		{
			auto& [glyph, destRect, _] = cache[j];

			assert(glyph.character != Glyph::kInvalidChar);

			destRect = { 
				static_cast<float>(xPos), 
				static_cast<float>(yPos), 
				static_cast<float>(glyph.plot.rect.w * format.layoutScale.x),
				static_cast<float>(glyph.plot.rect.h * format.layoutScale.y) 
			};

			xPos += static_cast<float>(glyph.advance) * format.layoutScale.x;
		}

		yPos += static_cast<float>(format.fontHeight) * format.layoutScale.y;
		currentTextPos = newlinePos + 1;
	}
} 

} // unnamed

void GlyphCacheHandler::RepopulateGlyphCacheGlyphs(std::string_view text,
	std::vector<GlyphCacheData>& cache, const FontAtlasTexture& glyphAtlas)
{
	cache.resize(text.size());

	for (size_t i = 0; i < text.size(); i++)
	{
		cache[i].destRect = { 0, 0, 0, 0 };
		cache[i].glyph = glyphAtlas.GetGlyph(text[i]);
	}
}

void GlyphCacheHandler::ReprojectGlyphCacheGeometry(TextRenderableGlyphCache& cacheComponent, 
													const TextRenderableComponent& textRenderable,
													const FontAtlasTexture& glyphAtlas)
{
	//auto formatArgs = MakeFormatArgs(textRenderable, cacheComponent,
	//								 glyphAtlas.GetFontHeight());

	//switch (textRenderable.formatting.align)
	//{
	//case TextAlign::Left:
	//	FillGlyphRectsLeftAlign(cacheComponent.cache, formatArgs);
	//	break;
	//case TextAlign::Right:
	//	FillGlyphRectsRightAlign(cacheComponent.cache, formatArgs);
	//	break;
	//case TextAlign::Center: default:
	//	FillGlyphRectsCenterAlign(textRenderable.writer.text, 
	//							  cacheComponent.cache, formatArgs);
	//	break;
	//}
	FillGlyphRects(cacheComponent.cache, glyphAtlas.GetFontHeight(), textRenderable.formatting);
}

void GlyphCacheHandler::RotateGlyphCache(std::vector<GlyphCacheData>& cache,
										 Anchor rotateAnchor, float angleDegrees)
{
	SDL_FRect textBlockBbox = ComputeGlyphDataBoundingBox(cache);
	SDL_FPoint textBlockPivotPoint = GetRectAnchorPoint(textBlockBbox, rotateAnchor);

	const float radians = angleDegrees * static_cast<float>(M_PI) / 180.0f;
	const float cosA = std::cos(radians);
	const float sinA = std::sin(radians);

	for (auto& [_, destRect, rotCenter] : cache)
	{
		SDL_FPoint center = GetRectCenter(destRect);

		// vector from pivot -> glyph center
		float dx = center.x - textBlockPivotPoint.x;
		float dy = center.y - textBlockPivotPoint.y;

		// rotate that vector
		float rx = dx * cosA - dy * sinA;
		float ry = dx * sinA + dy * cosA;

		SDL_FPoint newCenter{ textBlockPivotPoint.x + rx, textBlockPivotPoint.y + ry };

		// write back (keep current width/height)
		destRect.x = newCenter.x - destRect.w * 0.5f;
		destRect.y = newCenter.y - destRect.h * 0.5f;

		// rotation center remains glyph-local center (already set by ScaleGlyphCache)
		// but keep in sync in case sizes changed:
		rotCenter.x = destRect.w * 0.5f;
		rotCenter.y = destRect.h * 0.5f;
	}
}

void GlyphCacheHandler::ScaleGlyphCache(std::vector<GlyphCacheData>& cache, Anchor scaleAnchor,
										SDL_FPoint tfScale)
{
	SDL_FRect textBlockBbox = ComputeGlyphDataBoundingBox(cache);
	SDL_FPoint textBlockPivotPoint = GetRectAnchorPoint(textBlockBbox, scaleAnchor);

	// pivot is in local block-space; scale everything relative to it
	for (auto& [_, destRect, rotCenter] : cache)
	{
		SDL_FPoint oldDestRectCenter = GetRectCenter(destRect);

		// offset from pivot
		float dx = oldDestRectCenter.x - textBlockPivotPoint.x;
		float dy = oldDestRectCenter.y - textBlockPivotPoint.y;

		// apply transform.scale
		float newDestRectCenterX = textBlockPivotPoint.x + dx * tfScale.x;
		float newDestRectCenterY = textBlockPivotPoint.y + dy * tfScale.y;

		float newW = destRect.w * tfScale.x;
		float newH = destRect.h * tfScale.y;

		// update dest rect & rotCenter
		destRect.w = newW;
		destRect.h = newH;
		destRect.x = newDestRectCenterX - newW * 0.5f;
		destRect.y = newDestRectCenterY - newH * 0.5f;

		rotCenter.x = newW * 0.5f; 
		rotCenter.y = newH * 0.5f;	
	}
}

void GlyphCacheHandler::RepositionGlyphCache(TextRenderableGlyphCache& cacheComponent,
											 SDL_FPoint newPos, SDL_FPoint newOffset)
{
	SDL_FPoint adjust = (newPos - cacheComponent.context.transform.position) +
						(newOffset - cacheComponent.context.offset);

	for (auto& cacheData : cacheComponent.cache)
	{
		cacheData.destRect.x += adjust.x;
		cacheData.destRect.y += adjust.y;
	}
}

void GlyphCacheHandler::UpdateGlyphCache(const FontAtlasTexture& glyphAtlas,
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

	// 1) Update text hash if needed
	if (changeLog & TextChanged)
	{
		ctx.textHash = RapidHash(textRenderable.writer.text);
	}

	// 2) Rebuild glyphs
	static constexpr uint8_t kNeedsRepopulate = (AtlasChanged | TextChanged);
	if (changeLog & kNeedsRepopulate)
	{
		RepopulateGlyphCacheGlyphs(textRenderable.writer.text, glyphCache.cache, glyphAtlas);
	}

	// 3) Layout (unscaled local block-space) if formatting or glyph set changed
	static constexpr uint8_t kNeedsReprojection = (FormatChanged | kNeedsRepopulate);
	if (changeLog & kNeedsReprojection)
	{
		ReprojectGlyphCacheGeometry(glyphCache, textRenderable, glyphAtlas);
		ctx.transform = {};
		ctx.offset = { 0.0f, 0.0f };
	}

	// 4) Apply transform.scale if scale changed OR if reprojection produced new layout
	static constexpr uint8_t kNeedsScaling = (ScaleChanged | kNeedsReprojection);
	if (changeLog & kNeedsScaling) 
	{
		assert(ctx.transform.scale.x > 0.0f && ctx.transform.scale.y > 0.0f);
		const SDL_FPoint deltaScale = GetClampedScale(transform) / ctx.transform.scale;
		ScaleGlyphCache(glyphCache.cache, textRenderable.profile.anchor.scale, deltaScale);
	}

	// 5) Rotate if rotation changed
	static constexpr uint8_t kNeedsRotate = (RotationChanged | kNeedsScaling);
	if (changeLog & kNeedsRotate)
	{
		const float deltaRotation = transform.rotation - ctx.transform.rotation;
		RotateGlyphCache(glyphCache.cache, textRenderable.profile.anchor.rotation, deltaRotation);
	}

	// 6) Reposition (translate to world) if position or offset changed,
	//    or if any of reprojection/scale/rotation happened (we need to re-place in world space)
	static constexpr uint8_t kNeedsReposition = (PositionChanged | kNeedsRotate);
	if (changeLog & kNeedsReposition)
	{
		RepositionGlyphCache(glyphCache, transform.position, textRenderable.profile.offset);
	}

	// 6) update rest of cached content
	ctx.transform = transform;
	ctx.transform.scale = GetClampedScale(transform);

	ctx.formatting = textRenderable.formatting; 
	ctx.offset = textRenderable.profile.offset;
	ctx.resourceHandle = textRenderable.writer.resourceHandle;
}