#include <algorithm>
#include "RenderSystem.h"
#include "../ecs/Ecs.h"
#include "../atlas/GlyphAtlas.h"
//#include "../atlas/AtlasManager.h"
//#include "../components/RenderableComponent.h"

namespace
{
using Alignment = Renderable::Text::Alignment;

template <Alignment T>
void RenderGlyphsAligned(const RenderSystem::RenderGlyphsArgs& args);

template <>
void RenderGlyphsAligned<Alignment::Left>(const RenderSystem::RenderGlyphsArgs& args)
{
	int xPos = args.startX;
	int yPos = args.startY;

	for (size_t i = 0; i < args.text.size(); i++)
	{
		if (args.text[i] == '\n')
		{
			xPos = args.startX;
			yPos += args.glyphAtlas->GetAtlasInfo().fontHeight;

			continue;
		}

		auto glyph = args.glyphAtlas->GetGlyph(args.text[i]);
		assert(glyph.character != kInvalidChar);

		SDL_Rect dest = { xPos, yPos, static_cast<int>(glyph.atlasRect.w * args.scale.x),
									  static_cast<int>(glyph.atlasRect.h * args.scale.y) };

		SDL_RenderCopy(args.renderer, args.glyphAtlas->GetAtlasTexture(),
			&glyph.atlasRect, &dest);

		xPos += static_cast<int>(glyph.advance * args.scale.x);
	}
}

template <>
void RenderGlyphsAligned<Alignment::Right>(const RenderSystem::RenderGlyphsArgs& args)
{
	int xPos = args.startX;
	int yPos = args.startY;

	for (int i = args.text.size() - 1; i >= 0; i--)
	{
		if (args.text[i] == '\n')
		{
			xPos = args.startX;
			yPos += args.glyphAtlas->GetAtlasInfo().fontHeight;

			continue;
		}

		auto glyph = args.glyphAtlas->GetGlyph(args.text[i]);
		assert(glyph.character != kInvalidChar);

		SDL_Rect dest = { xPos - static_cast<int>(glyph.atlasRect.w * args.scale.x), yPos,
						  static_cast<int>(glyph.atlasRect.w * args.scale.x),
						  static_cast<int>(glyph.atlasRect.h * args.scale.y) };

		SDL_RenderCopy(args.renderer, args.glyphAtlas->GetAtlasTexture(),
			&glyph.atlasRect, &dest);

		xPos -= static_cast<int>(glyph.advance * args.scale.x);
	}
}

template <>
void RenderGlyphsAligned<Alignment::Center>(const RenderSystem::RenderGlyphsArgs& args)
{
	auto glyphs = args.glyphAtlas->GetGlyphsForString(args.text);
	assert(glyphs.size() == args.text.size());

	int yPos = args.startY;
	int currentPos = 0;
	for (int i = 0; i <= args.numNewlines; i++)
	{
		size_t newlinePos = args.text.find_first_of('\n', currentPos);
		newlinePos = (std::min(newlinePos, args.text.length()));

		int rowWidth = std::accumulate(
			glyphs.begin() + currentPos,
			glyphs.begin() + newlinePos,
			0, [scale = args.scale.x](int sum, const auto& glyph) {
				return sum + static_cast<int>(glyph.advance * scale);
			});

		int xPos = args.startX - static_cast<int>(rowWidth / 2.0f);

		for (size_t j = currentPos; j < newlinePos; j++)
		{
			auto& glyph = glyphs[j];
			assert(glyph.character != kInvalidChar);

			SDL_Rect dest = { xPos , yPos, static_cast<int>(glyph.atlasRect.w * args.scale.x),
										   static_cast<int>(glyph.atlasRect.h * args.scale.y) };

			SDL_RenderCopy(args.renderer, args.glyphAtlas->GetAtlasTexture(),
				&glyph.atlasRect, &dest);

			xPos += static_cast<int>(glyph.advance * args.scale.x);
		}

		yPos += static_cast<int>(args.glyphAtlas->GetAtlasInfo().fontHeight * args.scale.y);
		currentPos = newlinePos + 1;
	}
}

static SDL_Rect MakeTransformedRect(const Spatial& spatial, const Transform& tf)
{
	float scaledW = spatial.dimensions.w * tf.scale.x;
	float scaledH = spatial.dimensions.h * tf.scale.y;

	return SDL_Rect{
		static_cast<int>(spatial.position.x - (scaledW / 2.0f)),
		static_cast<int>(spatial.position.y - (scaledH / 2.0f)),
		static_cast<int>(scaledW),
		static_cast<int>(scaledH)
	};
}

} // unnamed namespace


// TODO: Decide what to do about entities w/o transform
void RenderSystem::Update(SDL_Renderer* renderer, const impl::AtlasStore& atlasStore)
{
	auto entities = ECS::GetAllEntitiesWith<Spatial, Renderable>();

	std::sort(entities.begin(), entities.end(), [](const Entity& lhs, const Entity& rhs) {
		return lhs.GetComponent<Renderable>().drawOrder <
			   rhs.GetComponent<Renderable>().drawOrder;
		});

	for (auto& entity : entities)
	{
		auto& renderable = entity.GetComponent<Renderable>();
		auto& spatial = entity.GetComponent<Spatial>();

		Transform transform = (entity.HasComponent<Transform>()) ?
			entity.GetComponent<Transform>() : Transform{};

		SDL_Rect renderRect = MakeTransformedRect(spatial, transform);

		if (auto spriteData = std::get_if<Renderable::Sprite>(&renderable.renderData))
		{
			auto spriteAtlas = atlasStore.GetAtlas(spriteData->sourceAtlas);
			if (!spriteAtlas)
			{
				std::cerr << "Sprite handle expired for sprite { seriesName : " << spriteData->seriesName
					<< ", index : " << spriteData->currentIndex << " }\n";

				continue;
			}

			SDL_Rect srcRect = spriteAtlas->GetSprite(spriteData->seriesName,
													  spriteData->currentIndex).atlasRect;
			if (srcRect.w == 0 || srcRect.h == 0)
			{
				std::cerr << "SpriteInfo not found for sprite { seriesName : " << spriteData->seriesName
					<< ", index : " << spriteData->currentIndex << " }\n";

				continue;
			}

			SDL_RenderCopyEx(renderer, spriteAtlas->GetAtlasTexture(), &srcRect,
				&renderRect, transform.rotation, nullptr, renderable.flip);
		}

		else if (auto textData = std::get_if<Renderable::Text>(&renderable.renderData))
		{
			if (textData->text.empty()) { continue; }

			auto glyphAtlas = atlasStore.GetAtlas(textData->sourceAtlas);
			if (!glyphAtlas)
			{
				std::cerr << "Glyph handle expired for text { \"" << textData->text << " }\n";

				continue;
			}

			const int numNewlines = std::count(textData->text.begin(), textData->text.end(), '\n');
			const int totalHeight = 
				static_cast<int>(glyphAtlas->GetAtlasInfo().fontHeight * transform.scale.y) * (numNewlines + 1);

			RenderGlyphsArgs args{
				.renderer = renderer,
				.glyphAtlas = glyphAtlas,
				.text = textData->text,
				.scale = transform.scale,
				.numNewlines = numNewlines,
				.startY = static_cast<int>(spatial.position.y - (totalHeight / 2.0f))
			};

			if (textData->scaleToFit)
			{
				float toFit = GetScaleToFitFactor(args, totalHeight, renderRect.w, renderRect.h);

				args.scale.x *= toFit;
				args.scale.y *= toFit;
			}

			switch (textData->align)
			{
			case Alignment::Left:
				args.startX = renderRect.x;
				RenderGlyphsAligned<Alignment::Left>(args);
				break;

			case Alignment::Right:
				args.startX = renderRect.x + renderRect.w;
				RenderGlyphsAligned<Alignment::Right>(args);
				break;

			case Alignment::Center:
				args.startX = static_cast<int>(spatial.position.x);
				RenderGlyphsAligned<Alignment::Center>(args);
				break;
			}
		}

		// TODO: Add fill/line option for how to draw
		else if (auto geometryData = std::get_if<Renderable::Geometry>(&renderable.renderData))
		{
			auto origColor = GetRenderDrawColor(renderer);
			SetRenderDrawColor(renderer, geometryData->color);

			SDL_RenderFillRect(renderer, &renderRect);

			SetRenderDrawColor(renderer, origColor);
		}

		else
		{
			std::cerr << "Logic error: renderable type not text or sprite";
		}
	}
}

float RenderSystem::GetScaleToFitFactor(const RenderGlyphsArgs& args, int totalHeight,
										int boundingWidth, int boundingHeight)
{
	assert(args.glyphAtlas);

	auto glyphs = args.glyphAtlas->GetGlyphsForString(args.text);
	assert(glyphs.size() == args.text.size());

	int currentPos = 0;
	int longestRowWidth = 0;
	for (int i = 0; i <= args.numNewlines; i++)
	{
		size_t newlinePos = args.text.find_first_of('\n', currentPos);
		newlinePos = (std::min(newlinePos, args.text.length()));

		int rowWidth = std::accumulate(
			glyphs.begin() + currentPos,
			glyphs.begin() + newlinePos,
			0, [scale = args.scale.x](int sum, const auto& glyph) {
				return sum + (glyph.advance * scale);
			});

		longestRowWidth = std::max(longestRowWidth, rowWidth);

		currentPos = newlinePos + 1;
	}

	return std::min(boundingWidth / static_cast<float>(longestRowWidth),
		boundingHeight / static_cast<float>(totalHeight));
}

//static SDL_Rect SpatialToRect(const Spatial& spatial)
//{
//    return SDL_Rect{
//        static_cast<int>(spatial.position.x - (spatial.dimensions.w / 2.0f)),
//        static_cast<int>(spatial.position.y - (spatial.dimensions.h / 2.0f)),
//        static_cast<int>(spatial.dimensions.w),
//        static_cast<int>(spatial.dimensions.h)
//    };
//}