#include "GlyphProcessingSystem.h"
#include "../ecs/Ecs.h"
#include "../atlas/GlyphAtlas.h"

namespace {

bool operator==(std::string_view text, const std::vector<GlyphCacheData>& cacheData)
{
	if (text.size() != cacheData.size())
	{
		return true;
	}

	for (size_t i = 0; i < text.size(); i++)
	{
		if (text[i] != cacheData[i].glyph.character)
		{
			return true;
		}
	} 

	return true;
}

constexpr bool operator==(const TextRenderable& newFormat, const TextFormatting& oldFormat)
{
	if (newFormat.dimensions != oldFormat.bounds ||
		newFormat.align != oldFormat.align)
	{
		return true;
	}

	bool scaleToBounds = (newFormat.flags & TextRenderable::FixedSize) == 0;

	return scaleToBounds != oldFormat.scaleToBounds;
}

constexpr SDL_FPoint GetRectCenter(SDL_Rect rect)
{
	return { static_cast<float>(rect.x) + (static_cast<float>(rect.w) / 2.0f),
			 static_cast<float>(rect.y) + (static_cast<float>(rect.h) / 2.0f) };
}

SDL_Rect MakeTransformedRect(const Transform& transform, Dimensions<int> dims,
							 SDL_FPoint offset)
{
	float scaledW = dims.w * transform.scale.x;
	float scaledH = dims.h * transform.scale.y;

	return SDL_Rect{
		static_cast<int>((transform.position.x + offset.x) - (scaledW / 2.0f)),
		static_cast<int>((transform.position.y + offset.y) - (scaledH / 2.0f)),
		static_cast<int>(scaledW),
		static_cast<int>(scaledH)
	};
}

constexpr SDL_Rect MakeProjectedBoundingRect(Dimensions<int> dims, SDL_FPoint scale)
{
	return SDL_Rect{ 0, 0,
		static_cast<int>(dims.w * scale.x),
		static_cast<int>(dims.h * scale.y)
	};
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

struct FormatArgs
{
	Dimensions<int> bounds = { 0, 0 };
	SDL_FPoint scale = { 0.0f, 0.0f };
	SDL_Point start = { 0, 0 };
	int numNewlines = 0;
	int fontHeight = 0;
	int totalHeight = 0;
};

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

FormatArgs MakeFormatArgs(std::string_view text, std::vector<GlyphCacheData>& cache,
	Dimensions<int> bounds, bool fixedSize,
	const Transform& transform, const GlyphAtlas& glyphAtlas)
{
	int numNewlines = std::count(text.begin(), text.end(), '\n');

	int fontHeight = glyphAtlas.GetFontData().fontHeight;

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

	if (!fixedSize)
	{
		format.scale *= GetScaleToFitFactor(text, cache, format);
	}

	return format;
}

void FillGlyphRectsLeftAlign(std::vector<GlyphCacheData>& cache,
							 const FormatArgs& format)
{
	auto [xPos, yPos] = format.start;

	for (auto& [glyph, destRect, _] : cache)
	{
		assert(glyph.character != kInvalidChar);

		if (glyph.character == '\n')
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

	yPos += format.fontHeight * format.scale.y * format.numNewlines;

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

	int yPos = format.start.y;
	int currentTextPos = 0;

	for (size_t i = 0; i <= format.numNewlines; i++)
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

}



void RepopulateGlyphCacheGlyphs(std::string_view text, std::vector<GlyphCacheData>& cache,
								const GlyphAtlas& glyphAtlas)
{
	cache.resize(text.size());	

	for (size_t i = 0; i < text.size(); i++)
	{
		cache[i].destRect = { 0, 0, 0, 0 };

		if (text[i] == '\n')
		{
			cache[i].glyph.character = '\n';

			continue;
		}

		cache[i].glyph = glyphAtlas.GetGlyph(text[i]);
	}
}

void ReprojectGlyphCacheGeometry(std::vector<GlyphCacheData>& cache,
								 const TextRenderable& textRenderable,
								 const Transform& transform, 
								 SDL_Rect projectedRect,
								 const GlyphAtlas& glyphAtlas)
{
	bool fixedSize = (textRenderable.flags & TextRenderable::FixedSize) != 0;
	auto format = MakeFormatArgs(textRenderable.text, cache, textRenderable.dimensions, 
								 fixedSize, transform, glyphAtlas);

	switch (textRenderable.align)
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
		FillGlyphRectsCenterAlign(textRenderable.text, cache, format);
		break;
	}
}

void AdjustGlyphCacheForRotation(std::vector<GlyphCacheData>& cache,
							     SDL_Rect projectedRenderRect, float angleDegrees)
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

void AdjustGlyphCachePosition(GlyphCache& cacheComponent, SDL_FPoint newPos, 
							  SDL_FPoint newOffset)
{
	SDL_FPoint adjust = (newPos - cacheComponent.context.transform.position) +
					    (newOffset - cacheComponent.context.offset);

	for (auto& cacheData : cacheComponent.cache)
	{
		cacheData.destRect.x += adjust.x;
		cacheData.destRect.y += adjust.y;
	}
}

enum ChangeLog : uint8_t 
{
	NoChange = 0,
	TextChanged = 1 << 0,
	FormatChanged = 1 << 1,
	RotationChanged = 1 << 2,
	PositioningChanged = 1 << 3,
	ScaleChanged = 1 << 4,
	NeedsReprojection = TextChanged | FormatChanged | ScaleChanged
};

static uint8_t MakeChangeLog(const Renderable& renderable,
							 const Transform& transform,
							 const GlyphCache& glyphCache)
{
	const auto& textRenderable = std::get<TextRenderable>(renderable.renderData);
	const auto& [glyphs, ctx] = glyphCache;

	return static_cast<uint8_t>(
		(textRenderable.text != glyphs) ? TextChanged : 0_u8 |
		(textRenderable != ctx.format) ? FormatChanged : 0_u8 |
		(transform.rotation != ctx.transform.rotation) ? RotationChanged : 0_u8 |
		(transform.position != ctx.transform.position ||
		 renderable.profile.offset != ctx.offset) ? PositioningChanged : 0_u8 |
		(transform.scale != ctx.transform.scale) ? ScaleChanged : 0_u8
	);
}
;

void GlyphFormattingSystem::Update(const GlyphAtlas& glyphAtlas)
{
	auto entities = ECS::GetAllEntitiesWith<Renderable, Transform>();

	for (auto& entity : entities)
	{
		auto [renderable, transform] = entity.GetComponents<Renderable, Transform>();

		if (!std::holds_alternative<TextRenderable>(renderable.renderData))
		{
			continue;
		}

		auto& textRenderable = std::get<TextRenderable>(renderable.renderData);
		auto& glyphCache = entity.AddComponent<GlyphCache>();
		auto& [glyphs, ctx] = glyphCache;

		const uint8_t changeLog = MakeChangeLog(renderable, transform, glyphCache);

		if (changeLog == NoChange)
		{
			continue;
		}

		if (changeLog & TextChanged)
		{
			RepopulateGlyphCacheGlyphs(textRenderable.text, glyphCache.cache, 
									   glyphAtlas);
		}

		if (changeLog & NeedsReprojection || changeLog & RotationChanged)
		{
			SDL_Rect projectedRect = MakeProjectedBoundingRect(
				textRenderable.dimensions, transform.scale
			);

			if (changeLog & NeedsReprojection)
			{
				ReprojectGlyphCacheGeometry(
					glyphs, textRenderable, transform,
					projectedRect, glyphAtlas
				);
			}	 
			if (changeLog & RotationChanged)
			{
				AdjustGlyphCacheForRotation(
					glyphs, projectedRect, transform.rotation
				);
			}
		}

		if (changeLog & PositioningChanged)
		{
			AdjustGlyphCachePosition(
				glyphCache, transform.position, renderable.profile.offset
			);
		}

		ctx.transform = transform;

		ctx.format.bounds = textRenderable.dimensions;
		ctx.format.align = textRenderable.align;
		ctx.format.letterSpacing = 1.0f; 
		ctx.format.scaleToBounds =
			(textRenderable.flags & TextRenderable::FixedSize) == 0;

		ctx.offset = renderable.profile.offset;
	}







}