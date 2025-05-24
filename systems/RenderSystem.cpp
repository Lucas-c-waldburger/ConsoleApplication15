#include <algorithm>
#include "RenderSystem.h"
#include "../ecs/Ecs.h"
#include "../atlas/GlyphAtlas.h"
#include "../physics/B2Shape.h"
#include "../camera/Camera.h"
//#include "../atlas/AtlasManager.h"
//#include "../components/RenderableComponent.h"

namespace
{
//void RenderTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect& srcRect, 
//				   const SDL_Rect& destRect, const Renderable& renderable, const Transform& transform)
//{
//	if (renderable.opacity < 255)
//	{
//		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
//		SDL_SetTextureAlphaMod(texture, renderable.opacity);
//	}
//
//	SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, transform.rotation, nullptr, renderable.flip);
//
//	if (renderable.opacity < 255)
//	{
//		SDL_SetTextureAlphaMod(texture, 255);
//	}
//}
//
//void RenderTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect& srcRect, 
//				   const SDL_Rect& destRect, const Renderable& renderable)
//{
//	if (renderable.opacity < 255)
//	{
//		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
//		SDL_SetTextureAlphaMod(texture, renderable.opacity);
//	}
//
//	SDL_RenderCopy(renderer, texture, &srcRect, &destRect);
//
//	if (renderable.opacity < 255)
//	{
//		SDL_SetTextureAlphaMod(texture, 255);
//	}
//}

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

std::vector<SDL_FPoint> MakeCirclePerimeterPoints(SDL_FPoint center, float radius)
{
	std::vector<SDL_FPoint> points;
	points.reserve(8 * static_cast<int>(radius));

	int x = static_cast<int>(radius);
	int y = 0;
	int radiusError = 1 - x;

	while (x >= y)
	{
		points.push_back({ center.x + x, center.y - y }); // Top-right
		points.push_back({ center.x + y, center.y - x }); // Top-left
		points.push_back({ center.x - x, center.y - y }); // Bottom-left
		points.push_back({ center.x - y, center.y - x }); // Bottom-right
		points.push_back({ center.x - x, center.y + y }); // Bottom-left (mirrored)
		points.push_back({ center.x - y, center.y + x }); // Bottom-right (mirrored)
		points.push_back({ center.x + x, center.y + y }); // Top-right (mirrored)
		points.push_back({ center.x + y, center.y + x }); // Top-left (mirrored)

		y++;

		// Adjust the radiusError based on the distance from the center
		if (radiusError < 0)
		{
			radiusError += 2 * y + 1;
		}
		else
		{
			x--;
			radiusError += 2 * (y - x + 1);
		}
	}

	return points;
}

SDL_Rect MakeTransformedRect(const Transform& transform, int w, int h)
{
	float scaledW = w * transform.scale.x;
	float scaledH = h * transform.scale.y;

	return SDL_Rect{
		static_cast<int>(transform.position.x - (scaledW / 2.0f)),
		static_cast<int>(transform.position.y - (scaledH / 2.0f)),
		static_cast<int>(scaledW),
		static_cast<int>(scaledH)
	};
}

SDL_Rect MakeTransformedRect(const Transform& transform, Dimensions<int> dimensions)
{
	return MakeTransformedRect(transform, dimensions.w, dimensions.h);
}

SDL_Rect MakeScreenRect(const Camera& camera, const Transform& transform, int w, int h)
{
	SDL_Point screenPos = camera.WorldToScreen<SDL_Point>(transform.position);

	float scaledW = w * transform.scale.x;
	float scaledH = h * transform.scale.y;

	return SDL_Rect{
		static_cast<int>(screenPos.x - (scaledW / 2.0f)),
		static_cast<int>(screenPos.y - (scaledH / 2.0f)),
		static_cast<int>(scaledW),
		static_cast<int>(scaledH)
	};
}

SDL_Rect MakeScreenRect(const Camera& camera, const Transform& transform, Dimensions<int> dimensions)
{
	return MakeScreenRect(camera, transform, dimensions.w, dimensions.h);
}

Result<Void> DrawB2ColliderShape(const Camera& camera, SDL_Renderer* renderer, const Collider& collider)
{
	const auto& shapeData = collider.shape.GetData();

	assert(shapeData.IsValid());

	// test visible
	auto viewport = camera.GetViewport();
	if (!viewport.IntersectsBoundingBox(collider.shape.GetData().GetBoundingBox()))
	{
		return Void{};
	}

	auto toScreen = [&camera](const auto& p) { return camera.WorldToScreen<SDL_FPoint>(p); };

	switch (shapeData.GetShapeType())
	{
	case B2Shape::Type::Polygon: 
	{
		auto polyShape = shapeData.GetAs<B2PolygonShape>();

		auto verts = polyShape.GetVertices();

		std::transform(verts.begin(), verts.end(), verts.begin(), toScreen);
		
		SDL_RenderDrawLinesF(renderer, verts.data(), verts.size());
		
		break;
	}
	case B2Shape::Type::Circle:
	{
		auto circleShape = shapeData.GetAs<B2CircleShape>();

		SDL_FPoint center = circleShape.GetCenter();
		float radius = circleShape.GetRadius();

		auto points = MakeCirclePerimeterPoints(center, radius);

		std::transform(points.begin(), points.end(), points.begin(), toScreen);

		SDL_RenderDrawPointsF(renderer, points.data(), points.size());
		
		break;
	}

	case B2Shape::Type::Invalid: default:
		return MAKE_ERROR("Unsupported B2ShapeType");
	}

	return Void{};
}



} // unnamed namespace

void RenderSystem::Update(SDL_Renderer* renderer, const Camera& camera, const impl::TextureManager& atlasStore)
{
	auto entities = ECS::GetAllEntitiesWith<Renderable, Transform>();

	std::sort(entities.begin(), entities.end(), [](const Entity& lhs, const Entity& rhs) {
		return lhs.GetComponent<Renderable>().drawOrder <
			   rhs.GetComponent<Renderable>().drawOrder;
		});

	for (auto& entity : entities)
	{
		const auto& renderable = entity.GetComponent<Renderable>();
		const auto& transform = entity.GetComponent<Transform>();

		if (auto spriteData = std::get_if<Renderable::Sprite>(&renderable.renderData))
		{
			auto spriteAtlas = atlasStore.GetAtlas(spriteData->sourceAtlas);
			if (!spriteAtlas)
			{
				LOG_WARNING_FMT("Sprite handle expired for sprite (seriesName: '{}', index: '{}')",
							    spriteData->seriesName, spriteData->currentIndex);			 
				continue;
			}

			SDL_Rect srcRect = spriteAtlas->GetSprite(spriteData->seriesName,
													  spriteData->currentIndex).atlasRect;
			if (srcRect.w == 0 || srcRect.h == 0)
			{
				LOG_WARNING_FMT("SpriteInfo not found for sprite (seriesName: '{}', index: '{}')",
							    spriteData->seriesName, spriteData->currentIndex);
				continue;
			}

			//SDL_Rect renderRect = MakeTransformedRect(transform, srcRect.w, srcRect.h);
			SDL_Rect renderRect = MakeScreenRect(camera, transform, srcRect.w, srcRect.h);

			if (!camera.GetViewport().IntersectsBoundingBox(renderRect))
			{
				continue;
			}

			SDL_RenderCopyEx(renderer, spriteAtlas->GetAtlasTexture(), &srcRect,
							 &renderRect, transform.rotation, nullptr, renderable.flip);
		}

		// TODO : support non-overlay text that can move in world with camera
		else if (auto textData = std::get_if<Renderable::Text>(&renderable.renderData))
		{
			if (textData->text.empty()) { continue; }

			auto glyphAtlas = atlasStore.GetAtlas(textData->sourceAtlas);
			if (!glyphAtlas)
			{
				LOG_WARNING_FMT("Glyph handle expired for text '{}'", textData->text);
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
				.startY = static_cast<int>(transform.position.y - (totalHeight / 2.0f))
			};

			SDL_Rect renderRect = MakeTransformedRect(transform, textData->desiredDimensions);
			//SDL_Rect renderRect = MakeScreenRect(camera, transform, textData->desiredDimensions);

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
				args.startX = static_cast<int>(transform.position.x);
				RenderGlyphsAligned<Alignment::Center>(args);
				break;
			}
		}

		else if (auto geometryData = std::get_if<Renderable::Geometry>(&renderable.renderData))
		{
			if (!entity.HasComponent<Collider>())
			{
				LOG_ERROR("Entity with geometry render data did not have collider");
				continue;
			}

			auto& collider = entity.GetComponent<Collider>();
			if (!collider.shape.GetData().IsValid())
			{
				LOG_ERROR("Collider shape was invalid");
				continue;
			}

			auto origColor = GetRenderDrawColor(renderer);
			SetRenderDrawColor(renderer, geometryData->color);

			LOG_IF_ERROR(DrawB2ColliderShape(camera, renderer, collider));

			SetRenderDrawColor(renderer, origColor);
		}

		else
		{
			LOG_ERROR("Renderable type not recognized");
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
				return sum + static_cast<int>(glyph.advance * scale);
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