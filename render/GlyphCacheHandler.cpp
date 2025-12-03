#include "GlyphCacheHandler.h"
#include "../core/commonObjects.h"
#include "GlyphFormattingUtils.h"

using namespace util;

namespace {

template <SDLRectType R>
constexpr SDL_FPoint GetRectCenter(R rect)
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
		assert(glyph.character != Glyph::kInvalidChar);

		if (glyph == GlyphAtlas::kNewlineGlyph)
		{
			xPos = format.start.x;
			yPos += static_cast<int>(format.fontHeight * format.layoutScale.y);

			continue;
		}

		destRect = { xPos, yPos, static_cast<int>(glyph.plot.rect.w * format.layoutScale.x),
								 static_cast<int>(glyph.plot.rect.h * format.layoutScale.y) };

		xPos += static_cast<int>(glyph.advance * format.layoutScale.x);

	}
}

void FillGlyphRectsRightAlign(std::vector<GlyphCacheData>& cache,
							  const FormatArgs& format)
{
	auto [xPos, yPos] = format.start;

	yPos += static_cast<int>(format.fontHeight * format.layoutScale.y) * format.numNewlines;

	for (int i = static_cast<int>(cache.size()) - 1; i >= 0; i--)
	{
		auto& [glyph, destRect, _] = cache[i];

		assert(glyph.character != Glyph::kInvalidChar);

		if (glyph.character == '\n')
		{
			xPos = format.start.x;
			yPos -= static_cast<int>(format.fontHeight * format.layoutScale.y);

			continue;
		}

		xPos -= static_cast<int>(glyph.advance * format.layoutScale.x);

		destRect = { xPos, yPos, static_cast<int>(glyph.plot.rect.w * format.layoutScale.x),
								 static_cast<int>(glyph.plot.rect.h * format.layoutScale.y) };
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
			newlinePos, format.layoutScale.x);

		int xPos = format.start.x - static_cast<int>(rowWidth / 2.0f);

		for (size_t j = currentTextPos; j < newlinePos; j++)
		{
			auto& [glyph, destRect, _] = cache[j];

			assert(glyph.character != Glyph::kInvalidChar);

			destRect = { xPos , yPos, static_cast<int>(glyph.plot.rect.w * format.layoutScale.x),
									  static_cast<int>(glyph.plot.rect.h * format.layoutScale.y) };

			xPos += static_cast<int>(glyph.advance * format.layoutScale.x);
		}

		yPos += static_cast<int>(format.fontHeight * format.layoutScale.y);
		currentTextPos = newlinePos + 1;
	}
} 

} // unnamed

void GlyphCacheHandler::RepopulateGlyphCacheGlyphs(std::string_view text,
	std::vector<GlyphCacheData>& cache, const GlyphAtlas& glyphAtlas)
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
													const GlyphAtlas& glyphAtlas)
{
	//auto format = MakeFormatArgs(textRenderable.writer.text, cache,
	//							 textRenderable.formatting.bounds,
	//							 textRenderable.formatting.scaleToBounds,
	//							 glyphAtlas.GetFontDescriptor().fontHeight);
	auto formatArgs = MakeFormatArgs(textRenderable, cacheComponent,
									 glyphAtlas.GetFontDescriptor().fontHeight);

	switch (textRenderable.formatting.align)
	{
	case TextAlign::Left:
		//format.start.x = projectedRect.x;
		FillGlyphRectsLeftAlign(cacheComponent.cache, formatArgs);
		break;
	case TextAlign::Right:
		//format.start.x = projectedRect.x + projectedRect.w;
		FillGlyphRectsRightAlign(cacheComponent.cache, formatArgs);
		break;
	case TextAlign::Center: default:
		//format.start.x = projectedRect.x + (projectedRect.w / 2);
		FillGlyphRectsCenterAlign(textRenderable.writer.text, 
								  cacheComponent.cache, formatArgs);
		break;
	}
}

void GlyphCacheHandler::RotateGlyphCache(std::vector<GlyphCacheData>& cache,
										 Anchor rotateAnchor, float angleDegrees)
{
	SDL_FRect textBlockBbox = ComputeGlyphDataBoundingBox(cache);
	SDL_FPoint textBlockPivotPoint = GetRectAnchorPoint(textBlockBbox, rotateAnchor);

	static const float radians = angleDegrees * static_cast<float>(M_PI) / 180.0f;
	static const float cosA = std::cos(radians);
	static const float sinA = std::sin(radians);

	for (auto& [_, destRect, rotCenter] : cache)
	{
		SDL_FPoint center = GetRectCenter(destRect);

		// vector from pivot Å® glyph center
		float dx = center.x - textBlockPivotPoint.x;
		float dy = center.y - textBlockPivotPoint.y;

		// rotate that vector
		float rx = dx * cosA - dy * sinA;
		float ry = dx * sinA + dy * cosA;

		SDL_FPoint newCenter{ textBlockPivotPoint.x + rx, textBlockPivotPoint.y + ry };

		// write back (keep current width/height)
		destRect.x = static_cast<int>(newCenter.x - destRect.w * 0.5f);
		destRect.y = static_cast<int>(newCenter.y - destRect.h * 0.5f);

		// rotation center remains glyph-local center (already set by ScaleGlyphCache)
		// but keep in sync in case sizes changed:
		rotCenter.x = static_cast<int>(destRect.w * 0.5f);
		rotCenter.y = static_cast<int>(destRect.h * 0.5f);

		//SDL_Rect srcRect = glyph.plot.rect;

		//SDL_FPoint destCenter = GetRectCenter(destRect);

		//// Offset from bounding box center
		//float dx = destCenter.x - bboxCenter.x;
		//float dy = destCenter.y - bboxCenter.y;

		//// Rotate position
		//float rotatedX = dx * cosA - dy * sinA;
		//float rotatedY = dx * sinA + dy * cosA;

		//SDL_FPoint newPos = {
		//	bboxCenter.x + rotatedX,
		//	bboxCenter.y + rotatedY
		//};

		//// Adjust dest rect for new center
		//destRect.x = static_cast<int>(newPos.x - destRect.w / 2.0f);
		//destRect.y = static_cast<int>(newPos.y - destRect.h / 2.0f);

		//rotCenter = {
		//	static_cast<int>(destRect.w / 2.0f),
		//	static_cast<int>(destRect.h / 2.0f)
		//};
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

		int newW = static_cast<int>(destRect.w * tfScale.x);
		int newH = static_cast<int>(destRect.h * tfScale.y);

		// update dest rect & rotCenter
		destRect.w = newW;
		destRect.h = newH;
		destRect.x = static_cast<int>(newDestRectCenterX - newW * 0.5f);
		destRect.y = static_cast<int>(newDestRectCenterY - newH * 0.5f);

		rotCenter.x = static_cast<int>(newW * 0.5f); 
		rotCenter.y = static_cast<int>(newH * 0.5f);	
	}
}



void GlyphCacheHandler::RepositionGlyphCache(TextRenderableGlyphCache& cacheComponent,
											 SDL_FPoint newPos, SDL_FPoint newOffset)
{
	SDL_FPoint adjust = (newPos - cacheComponent.context.transform.position) +
						(newOffset - cacheComponent.context.offset);
	//SDL_FPoint adjust = newPos + newOffset;

	for (auto& cacheData : cacheComponent.cache)
	{
		cacheData.destRect.x += static_cast<int>(adjust.x);
		cacheData.destRect.y += static_cast<int>(adjust.y);
	}
}

void GlyphCacheHandler::UpdateGlyphCache(const GlyphAtlas& glyphAtlas,
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

	// 1) Rebuild glyphs
	static constexpr uint8_t kNeedsRepopulate = (AtlasChanged | TextChanged);
	if (changeLog & kNeedsRepopulate)
	{
		RepopulateGlyphCacheGlyphs(textRenderable.writer.text, glyphCache.cache, glyphAtlas);
	}

	// 2) Layout (unscaled local block-space) if formatting or glyph set changed
	static constexpr uint8_t kNeedsReprojection = (FormatChanged | kNeedsRepopulate);
	if (changeLog & kNeedsReprojection)
	{
		ReprojectGlyphCacheGeometry(glyphCache, textRenderable, glyphAtlas);
		ctx.transform.position = { 0.0f, 0.0f };
		ctx.offset = { 0.0f, 0.0f };
	}

	// 3) Apply transform.scale if scale changed OR if reprojection produced new layout
	//    We apply scale if either scale changed, or reprojection happened (which generated unscaled rects).
	static constexpr uint8_t kNeedsScaling = (ScaleChanged | kNeedsReprojection);
	if (changeLog & kNeedsScaling)
	{
		ScaleGlyphCache(glyphCache.cache, textRenderable.profile.anchor.scale, transform.scale);
	}

	// 4) Rotate if rotation changed
	if (changeLog & RotationChanged)
	{
		RotateGlyphCache(glyphCache.cache, textRenderable.profile.anchor.rotation, transform.rotation);
	}

	// 5) Reposition (translate to world) if position or offset changed,
	//    or if any of reprojection/scale/rotation happened (we need to re-place in world space)
	static constexpr uint8_t kNeedsReposition = (PositionChanged | RotationChanged | kNeedsScaling);
	if (changeLog & kNeedsReposition)
	{
		RepositionGlyphCache(glyphCache, transform.position, textRenderable.profile.offset);
	}

	// 6) update cached context
	ctx.transform = transform;
	ctx.formatting = textRenderable.formatting;
	ctx.offset = textRenderable.profile.offset;
	ctx.sourceAtlas = textRenderable.writer.sourceAtlas;
}


//void GlyphCacheHandler::UpdateGlyphCache(const NewGlyphAtlas& glyphAtlas,
//										 TextRenderableComponent& textRenderable,
//										 TextRenderableGlyphCache& glyphCache,
//										 const Transform& transform)
//{
//	uint8_t changeLog = MakeChangeLog(textRenderable, transform, glyphCache);
//	if (changeLog == NoChange)
//	{
//		return;
//	}
//
//	auto& [cache, ctx] = glyphCache;
//
//	if (changeLog & (AtlasChanged | TextChanged))
//	{
//		RepopulateGlyphCacheGlyphs(textRenderable.writer.text, cache, glyphAtlas);
//	}
//
//	if (changeLog & NeedsReprojection || changeLog & RotationChanged)
//	{
//		SDL_Rect projectedRect = { 0, 0,
//			static_cast<int>(textRenderable.formatting.bounds.w * transform.scale.x),
//			static_cast<int>(textRenderable.formatting.bounds.h * transform.scale.y)
//		};
//
//		if (changeLog & NeedsReprojection)
//		{
//			ReprojectGlyphCacheGeometry(cache, textRenderable, transform,
//										projectedRect, glyphAtlas);
//		}
//		if (changeLog & ScaleChanged)
//		{
//			ScaleGlyphCache(cache, textRenderable.profile.scaleAnchor, transform.scale);
//		}
//		if (changeLog & RotationChanged)
//		{
//			RotateGlyphCache(cache, projectedRect, transform.rotation);
//		}
//	}
//
//	if (changeLog & PositionChanged)
//	{
//		RepositionGlyphCache(glyphCache, transform.position, textRenderable.profile.offset);
//	}
//
//	ctx.transform = transform;
//	ctx.formatting = textRenderable.formatting;
//	ctx.offset = textRenderable.profile.offset;
//	ctx.sourceAtlas = textRenderable.writer.sourceAtlas;
//}