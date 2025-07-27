#include <algorithm>
#include "RenderSystem.h"
#include "../ecs/Ecs.h"
#include "../atlas/GlyphAtlas.h"
#include "../physics/B2Shape.h"
#include "../camera/Camera.h"


namespace
{

void Draw(SDL_Renderer* renderer, SDL_Texture* texture, const AtlasPlot& atlasPlot, SDL_Rect& destRect,
		  const Transform& transform, SDL_Point* rotationCenter, const RenderProfile& profile)
{
	SDL_RenderCopyEx(renderer, texture, &atlasPlot.rect, &destRect, 
					 static_cast<double>(transform.rotation + atlasPlot.rotation), rotationCenter, profile.flip);
}

constexpr bool HasRenderData(const Renderable& renderable)
{
	return !std::holds_alternative<std::monostate>(renderable.renderData);
}

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

//std::vector<size_t> GetNewLinePositions(std::string_view sv)
//{
//	std::vector<size_t> newlinePositions;
//	for (size_t i = 0; i < sv.size(); i++)
//	{
//		if (sv[i] == '\n')
//		{
//			newlinePositions.push_back(i);
//		}
//	}
//
//	return newlinePositions;
//}

int CalculateGlyphsRowWidth(const TextRenderable::GlyphCache& glyphCache, int currentPos,
							int newlinePos, float scaleX)
{
	return std::accumulate(
		glyphCache.begin() + currentPos,
		glyphCache.begin() + newlinePos,
		0, [scale = scaleX](int sum, const auto& data) {
			return sum + static_cast<int>(data.glyph.advance * scale);
		});
}

float GetScaleToFitFactor(const TextRenderable::GlyphCache& glyphCache,
						  const RenderSystem::TextRenderParams& params)
{
	int currentPos = 0;
	int longestRowWidth = 0;
	for (int i = 0; i <= params.numNewlines; i++)
	{
		size_t newlinePos = params.text.find_first_of('\n', currentPos);
		newlinePos = (std::min(newlinePos, params.text.length()));

		int rowWidth = CalculateGlyphsRowWidth(glyphCache, currentPos, 
											   newlinePos, params.scale.x);

		longestRowWidth = std::max(longestRowWidth, rowWidth);

		currentPos = newlinePos + 1;
	}

	return std::min(params.bounds.w / static_cast<float>(longestRowWidth),
					params.bounds.h / static_cast<float>(params.totalHeight));
}

void FillGlyphRectsLeftAlign(TextRenderable::GlyphCache& glyphCache,
							 const RenderSystem::TextRenderParams& params)
{
	int xPos = params.start.x;
	int yPos = params.start.y;

	for (auto& [glyph, destRect, _] : glyphCache)
	{
		assert(glyph.character != kInvalidChar);

		if (glyph.character == '\n')
		{
			xPos = params.start.x;
			yPos += static_cast<int>(params.fontHeight * params.scale.y);

			continue;
		}

		destRect = { xPos, yPos, static_cast<int>(glyph.plot.rect.w * params.scale.x),
								 static_cast<int>(glyph.plot.rect.h * params.scale.y) };

		xPos += static_cast<int>(glyph.advance * params.scale.x);

	}
}

void FillGlyphRectsRightAlign(TextRenderable::GlyphCache& glyphCache,
							  const RenderSystem::TextRenderParams& params)
{
	int xPos = params.start.x;
	int yPos = params.start.y + (params.fontHeight * params.scale.y * params.numNewlines);

	for (int i = glyphCache.size() - 1; i >= 0; i--)
	{
		auto& [glyph, destRect, _] = glyphCache[i];

		assert(glyph.character != kInvalidChar);

		if (glyph.character == '\n')
		{
			xPos = params.start.x;
			yPos -= static_cast<int>(params.fontHeight * params.scale.y);

			continue;
		}

		destRect = { xPos - static_cast<int>(glyph.plot.rect.w * params.scale.x), yPos,
					 static_cast<int>(glyph.plot.rect.w * params.scale.x),
					 static_cast<int>(glyph.plot.rect.h * params.scale.y) };

		xPos -= static_cast<int>(glyph.advance * params.scale.x);
	}
}

void FillGlyphRectsCenterAlign(TextRenderable::GlyphCache& glyphCache,
							   const RenderSystem::TextRenderParams& params)
{
	assert(params.text.size() == glyphCache.size());

	int yPos = params.start.y;
	int currentPos = 0;

	for (size_t i = 0; i <= params.numNewlines; i++)
	{
		size_t newlinePos = params.text.find_first_of('\n', currentPos);
		newlinePos = (std::min(newlinePos, params.text.length()));

		int rowWidth = CalculateGlyphsRowWidth(glyphCache, currentPos, 
											   newlinePos, params.scale.x);

		int xPos = params.start.x - static_cast<int>(rowWidth / 2.0f);

		for (size_t j = currentPos; j < newlinePos; j++)
		{
			auto& [glyph, destRect, _] = glyphCache[j];

			assert(glyph.character != kInvalidChar);

			destRect = { xPos , yPos, static_cast<int>(glyph.plot.rect.w * params.scale.x),
									  static_cast<int>(glyph.plot.rect.h * params.scale.y) };

			xPos += static_cast<int>(glyph.advance * params.scale.x);
		}

		yPos += static_cast<int>(params.fontHeight * params.scale.y);
		currentPos = newlinePos + 1;
	}
}

void RepopulateGlyphCacheGlyphs(TextRenderable& textRenderable, const GlyphAtlas* glyphAtlas)
{
	if (textRenderable.glyphCache.size() != textRenderable.text.size())
	{
		textRenderable.glyphCache.resize(textRenderable.text.size());
	}

	for (size_t i = 0; i < textRenderable.text.size(); i++)
	{
		char c = textRenderable.text[i];
		if (c == '\n')
		{
			textRenderable.glyphCache[i].glyph.character = '\n';

			continue;
		}

		textRenderable.glyphCache[i].glyph = glyphAtlas->GetGlyph(c);
	}

	auto glyphs = glyphAtlas->GetGlyphsForString(textRenderable.text);
}

void ReprojectGlyphCacheGeometry(TextRenderable& textRenderable, const Transform& transform,
								 const GlyphAtlas* glyphAtlas, SDL_Rect renderRect)
{
	// fill text params
	int numNewlines = std::count(textRenderable.text.begin(), 
								 textRenderable.text.end(), '\n');

	int fontHeight = glyphAtlas->GetFontData().fontHeight;

	int totalHeight = static_cast<int>(fontHeight * transform.scale.y) *
					  static_cast<int>(numNewlines + 1);

	RenderSystem::TextRenderParams textParams = {
		.text = textRenderable.text,
		.bounds = textRenderable.dimensions,
		.scale = transform.scale,
		.start = { 0, static_cast<int>(transform.position.y - (totalHeight / 2.0f)) },
		.numNewlines = numNewlines,
		.fontHeight = fontHeight,
		.totalHeight = totalHeight
	};

	float toFit = GetScaleToFitFactor(textRenderable.glyphCache, textParams);
	textParams.scale.x *= toFit;
	textParams.scale.y *= toFit;

	switch (textRenderable.align)
	{
	case TextAlign::Left:
		textParams.start.x = renderRect.x;
		FillGlyphRectsLeftAlign(textRenderable.glyphCache, textParams);
		break;
	case TextAlign::Right:
		textParams.start.x = renderRect.x + renderRect.w;
		FillGlyphRectsRightAlign(textRenderable.glyphCache, textParams);
		break;
	case TextAlign::Center: default:
		textParams.start.x = static_cast<int>(transform.position.x);
		FillGlyphRectsCenterAlign(textRenderable.glyphCache, textParams);
		break;
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

SDL_Rect MakeTransformedRect(const Transform& transform, int w, int h, 
							 SDL_FPoint offset = { 0.0f, 0.0f })
{
	float scaledW = w * transform.scale.x;
	float scaledH = h * transform.scale.y;

	return SDL_Rect{
		static_cast<int>((transform.position.x + offset.x) - (scaledW / 2.0f)),
		static_cast<int>((transform.position.y + offset.y) - (scaledH / 2.0f)),
		static_cast<int>(scaledW),
		static_cast<int>(scaledH)
	};
}

SDL_Rect MakeTransformedRect(const Transform& transform, Dimensions<int> dimensions, 
							 SDL_FPoint offset = { 0.0f, 0.0f })
{
	return MakeTransformedRect(transform, dimensions.w, dimensions.h, offset);
}

SDL_Rect MakeScreenRect(const Camera& camera, const Transform& transform, int w, int h,
					    SDL_FPoint offset = { 0.0f, 0.0f })
{
	SDL_Point screenPos = camera.WorldToScreen<SDL_Point>(transform.position + offset);

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

Result<Void> DrawB2ColliderShape(const Camera& camera, SDL_Renderer* renderer, const B2Shape& shapeData)
{
	assert(shapeData.IsValid());

	// test visible
	auto viewport = camera.GetViewport();
	if (!viewport.IntersectsBoundingBox(shapeData.GetBoundingBox()))
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

Result<Void> DrawB2ColliderShape(const Camera& camera, SDL_Renderer* renderer, const Collider& collider)
{
	return DrawB2ColliderShape(camera, renderer, collider.shape.GetData());
}

void SortByDrawOrder(std::vector<Entity>& entities)
{
	std::sort(entities.begin(), entities.end(), [](const Entity& lhs, const Entity& rhs) {
		return lhs.GetComponent<Renderable>().profile.drawOrder <
			   rhs.GetComponent<Renderable>().profile.drawOrder;
		});
}

constexpr void ExtractRectPoints(SDL_Rect rect, std::array<SDL_FPoint, 5>& points)
{
	points[0] = { static_cast<float>(rect.x), static_cast<float>(rect.y) };
	points[1] = { static_cast<float>(rect.x + rect.w), static_cast<float>(rect.y) };
	points[2] = { static_cast<float>(rect.x + rect.w), static_cast<float>(rect.y + rect.h) };
	points[3] = { static_cast<float>(rect.x), static_cast<float>(rect.y + rect.h) };
	points[4] = points[0];
}

constexpr void ExtractRectPoints(SDL_Rect rect, std::vector<SDL_FPoint>& points)
{
	if (points.size() < 5)
	{
		points.resize(5);
	}

	points[0] = { static_cast<float>(rect.x), static_cast<float>(rect.y) };
	points[1] = { static_cast<float>(rect.x + rect.w), static_cast<float>(rect.y) };
	points[2] = { static_cast<float>(rect.x + rect.w), static_cast<float>(rect.y + rect.h) };
	points[3] = { static_cast<float>(rect.x), static_cast<float>(rect.y + rect.h) };
	points[4] = points[0];
}

constexpr SDL_FPoint GetRectCenter(SDL_Rect rect)
{
	return { static_cast<float>(rect.x) + (static_cast<float>(rect.w) / 2.0f),
			 static_cast<float>(rect.y) + (static_cast<float>(rect.h) / 2.0f) };
}

void RotateRectPoints(std::array<SDL_FPoint, 5>& points, SDL_FPoint center, float angleDegrees)
{
	float radians = angleDegrees * (M_PI / 180.0f);
	float cosA = std::cos(radians);
	float sinA = std::sin(radians);

	for (int i = 0; i < points.size() - 1; ++i)
	{
		float dx = points[i].x - center.x;
		float dy = points[i].y - center.y;

		float x = dx * cosA - dy * sinA;
		float y = dx * sinA + dy * cosA;

		points[i].x = x + center.x;
		points[i].y = y + center.y;
	}

	points.back() = points.front();
}

void RotateRectPoints(std::vector<SDL_FPoint>& points, SDL_FPoint center, float angleDegrees)
{
	if (points.size() < 5)
	{
		points.resize(5);
	}

	float radians = angleDegrees * (M_PI / 180.0f);
	float cosA = std::cos(radians);
	float sinA = std::sin(radians);

	for (int i = 0; i < 4; ++i)
	{
		float dx = points[i].x - center.x;
		float dy = points[i].y - center.y;

		float x = dx * cosA - dy * sinA;
		float y = dx * sinA + dy * cosA;

		points[i].x = x + center.x;
		points[i].y = y + center.y;
	}

	points[4] = points[0];
}

SDL_Rect ComputeBoundingBox(const std::vector<SDL_Rect>& rects)
{
	if (rects.empty())
	{
		return { 0, 0, 0, 0 };
	}
		
	int minX = rects[0].x;
	int minY = rects[0].y;
	int maxX = rects[0].x + rects[0].w;
	int maxY = rects[0].y + rects[0].h;

	for (size_t i = 1; i < rects.size(); ++i)
	{
		minX = std::min(minX, rects[i].x);
		minY = std::min(minY, rects[i].y);
		maxX = std::max(maxX, rects[i].x + rects[i].w);
		maxY = std::max(maxY, rects[i].y + rects[i].h);
	}

	return { minX, minY, maxX - minX, maxY - minY };
}

void AdjustGlyphCacheForRotation(TextRenderable::GlyphCache& glyphCache,
								 SDL_Rect renderRect, float angleDegrees)
{
	SDL_FPoint bboxCenter = GetRectCenter(renderRect);

	float radians = angleDegrees * static_cast<float>(M_PI) / 180.0f;
	float cosA = std::cos(radians);
	float sinA = std::sin(radians);

	for (auto& [glyph, destRect, rotationCenter] : glyphCache)
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

		rotationCenter = { 
			static_cast<int>(destRect.w / 2.0f), 
			static_cast<int>(destRect.h / 2.0f)
		};
	}
}

void HandleBoundingBoxDebugDraw(RenderSystem::RenderContext& context, SDL_Rect renderRect, 
							    const RenderProfile& profile, float rotation)
{
	assert(profile.debugDraw.boundingBox.on);
	
	ExtractRectPoints(renderRect, context.debugDrawPoints);

	if (rotation != 0.0f)
	{
		RotateRectPoints(context.debugDrawPoints, GetRectCenter(renderRect), rotation);
	}

	SDL_Color bboxColor = profile.debugDraw.boundingBox.color;
	if (bboxColor != context.currentDrawColor)
	{
		SetRenderDrawColor(context.renderer, bboxColor);

		context.currentDrawColor = bboxColor;
	}

	SDL_RenderDrawLinesF(context.renderer, context.debugDrawPoints.data(),
							context.debugDrawPoints.size());
}

void HandleColliderDebugDraw(RenderSystem::RenderContext& context, const RenderProfile& profile,
							 const Collider& collider)
{
	assert(profile.debugDraw.collider.on);

	auto& colliderShape = collider.shape.GetData();
	if (!colliderShape.IsValid())
	{
		return;
	}

	SDL_Color colliderColor = profile.debugDraw.collider.color;
	if (colliderColor != context.currentDrawColor)
	{
		SetRenderDrawColor(context.renderer, colliderColor);

		context.currentDrawColor = colliderColor;
	}

	LOG_IF_ERROR(DrawB2ColliderShape(*context.camera, context.renderer, colliderShape));
}

} // unnamed


void RenderSystem::Update(SDL_Renderer* renderer, const Camera& camera, 
							 const TextureRepository& textureRepo)
{
	auto entities = ECS::GetAllEntitiesWith<Renderable>();

	SortByDrawOrder(entities);

	UpdateRenderContext(renderer, textureRepo, camera);
	SDL_Color originalDrawColor = context_.currentDrawColor;

	for (auto& entity : entities)
	{
		auto& renderable = entity.GetComponent<Renderable>();
		const auto& transform = entity.GetComponent<Transform>();

		if (HasRenderData(renderable))
		{
			if (auto* spriteData = std::get_if<SpriteRenderable>(&renderable.renderData))
			{
				RenderSprite(*spriteData, transform, renderable.profile);
			}
			else if (auto* textData = std::get_if<TextRenderable>(&renderable.renderData))
			{
				RenderText(*textData, transform, renderable.profile);
			}
			else
			{
				LOG_ERROR("Renderable's renderData could not be retrieved");
			}
		}

		if (renderable.profile.debugDraw.collider.on && entity.HasComponent<Collider>())
		{
			const auto& collider = entity.GetComponent<Collider>();

			HandleColliderDebugDraw(context_, renderable.profile, collider);
		}
	}

	if (originalDrawColor != context_.currentDrawColor)
	{
		SetRenderDrawColor(renderer, originalDrawColor);
	}
}

void RenderSystem::UpdateRenderContext(SDL_Renderer* renderer, const TextureRepository& textureRepo, 
										  const Camera& camera)
{
	context_.renderer = renderer;
	context_.textureRepo = &textureRepo;
	context_.camera = &camera;
	context_.currentDrawColor = GetRenderDrawColor(renderer);
}

void RenderSystem::RenderSprite(const SpriteRenderable& spriteRenderable, const Transform& transform,
								   const RenderProfile& renderProfile)
{
	SDL_Texture* atlasTexture = context_.textureRepo->GetAtlasTexture(spriteRenderable.sourceAtlas);
	if (!atlasTexture)
	{
		LOG_WARNING("Sprite series atlas handle expired");

		return;
	}

	SDL_Rect renderRect = MakeScreenRect(*context_.camera, transform, 
										 spriteRenderable.sourcePlot.rect.w,
										 spriteRenderable.sourcePlot.rect.h, 
										 renderProfile.offset);

	if (!context_.camera->GetViewport().IntersectsBoundingBox(renderRect))
	{
		return;
	}
	//LOG_DEBUG_FMT("Transform Rotation: {}\nAtlas Plot Rotation: {}\n\n",
	//	transform.rotation, spriteRenderable.sourcePlot.rotation);
	Draw(context_.renderer, atlasTexture, spriteRenderable.sourcePlot, 
		 renderRect, transform, nullptr, renderProfile);

	//SDL_RenderCopyEx(context_.renderer, atlasTexture, &spriteRenderable.sourcePlot,
	//				 &renderRect, transform.rotation, nullptr, renderProfile.flip);

	if (renderProfile.debugDraw.boundingBox.on)
	{
		HandleBoundingBoxDebugDraw(context_, renderRect, renderProfile, transform.rotation);
	}
}

void RenderSystem::RenderText(TextRenderable& textRenderable, const Transform& transform, 
								 const RenderProfile& renderProfile)
{
	if (textRenderable.text.empty() || 
		textRenderable.dimensions.w <= 0 || textRenderable.dimensions.h <= 0)
	{
		return; 
	}

	const GlyphAtlas* glyphAtlas = context_.textureRepo->GetAtlas(textRenderable.sourceAtlas);
	if (!glyphAtlas)
	{
		LOG_WARNING("glyph atlas handle expired");

		return;
	}

	// make world rect (renderRect) and screen rect, return early if not visible on screen
	SDL_Rect renderRect = MakeTransformedRect(transform, textRenderable.dimensions.w,
											  textRenderable.dimensions.h, renderProfile.offset);
	SDL_Point renderRectXY = { renderRect.x, renderRect.y };

	SDL_Point screenXY = context_.camera->WorldToScreen<SDL_Point>(renderRectXY);
	SDL_Point screenAdjust = renderRectXY - screenXY;

	SDL_Rect screenRect = {
		renderRect.x + screenAdjust.x,
		renderRect.y + screenAdjust.y,
		renderRect.w,
		renderRect.h
	};

	if (!context_.camera->GetViewport().IntersectsBoundingBox(screenRect))
	{
		return;
	}

	// Update glyph cache as needed
	using DirtyFlag = TextRenderable::DirtyFlag;
	if (textRenderable.dirtyFlags & DirtyFlag::NewText)
	{
		RepopulateGlyphCacheGlyphs(textRenderable, glyphAtlas);
	}
	if (textRenderable.dirtyFlags & (DirtyFlag::NewText | DirtyFlag::NewTransforms))
	{
		ReprojectGlyphCacheGeometry(textRenderable, transform, glyphAtlas, renderRect);

		// reprojecting geometry has them at 0 deg rotation, skip if no transform rotation
		//textRenderable.dirtyFlags = (transform.rotation != 0.0f) ? DirtyFlag::NewRotation : 0;
		if (transform.rotation != 0.0f)
		{
			AdjustGlyphCacheForRotation(textRenderable.glyphCache, renderRect, transform.rotation);
		}
	}	
	//if (textRenderable.dirtyFlags & DirtyFlag::NewRotation)
	//{
	//	AdjustGlyphCacheForRotation(textRenderable.glyphCache, renderRect, transform.rotation);
	//}

	textRenderable.dirtyFlags = 0;

	// adjust for screen projection and render
	for (const auto& [glyph, destRect, rotationCenter] : textRenderable.glyphCache)
	{
		SDL_Rect destRectScreenAdjusted = {
			destRect.x + screenAdjust.x,
			destRect.y + screenAdjust.y,
			destRect.w,
			destRect.h
		};

		SDL_Point rotationCenterScreenAdjusted = {
			rotationCenter.x + screenAdjust.x,
			rotationCenter.y + screenAdjust.y
		};

		//SDL_RenderCopyEx(context_.renderer, glyphAtlas->GetAtlasTexture(), &glyph.atlasRect, 
		//				 &destRectScreenAdjusted, transform.rotation, &rotationCenterScreenAdjusted,
		//				 renderProfile.flip);
		Draw(context_.renderer, glyphAtlas->GetAtlasTexture(), glyph.plot, destRectScreenAdjusted,
			 transform, &rotationCenterScreenAdjusted, renderProfile);
	}

	if (renderProfile.debugDraw.boundingBox.on)
	{
		HandleBoundingBoxDebugDraw(context_, screenRect, renderProfile, transform.rotation);
	}
}


