#include "DebugDrawUtils.h"
#include <SDL_render.h>
#include "../../camera/Camera.h"
#include "../../physics/B2Shape.h"
#include "../../components/ColliderComponent.h"
#include "../../components/RenderableComponent.h"
#include "../../core/Logger.h"

namespace util {

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



void DrawB2ColliderShape(const Camera& camera, SDL_Renderer* renderer, 
					     const B2Shape& shapeData)
{
	assert(shapeData.IsValid());

	if (!camera.GetViewport().IntersectsBoundingBox(shapeData.GetBoundingBox()))
	{
		return;
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
		LOG_ERROR("Unsupported B2ShapeType");
	}
}

void DrawB2ColliderShape(const Camera& camera, SDL_Renderer* renderer, 
						 const Collider& collider)
{
	DrawB2ColliderShape(camera, renderer, collider.shape.GetData());
}

void ExtractRectPoints(SDL_Rect rect, std::vector<SDL_FPoint>& points)
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


} // util


//
//void DebugDrawHandler::DrawBoundingBox(SDL_Renderer* renderer, SDL_Rect renderRect, 
//									   float rotation, SDL_Color color)
//{
//	util::ExtractRectPoints(renderRect, debugDrawPoints_);
//
//	if (rotation != 0.0f)
//	{
//		SDL_FPoint rectCenter{ 
//			static_cast<float>(renderRect.x) + (static_cast<float>(renderRect.w) / 2.0f),
//			static_cast<float>(renderRect.y) + (static_cast<float>(renderRect.h) / 2.0f)
//		};
//
//		util::RotateRectPoints(debugDrawPoints_, rectCenter, rotation);
//	}
//
//	if (color != currentDrawColor_)
//	{
//		SetRenderDrawColor(renderer, color);
//
//		currentDrawColor_ = color;
//	}
//
//	SDL_RenderDrawLinesF(renderer, debugDrawPoints_.data(), debugDrawPoints_.size());
//}
//
//void DebugDrawHandler::DrawColliderShape(SDL_Renderer* renderer, const Camera& camera, 
//										 const Collider& collider, SDL_Color color)
//{
//	auto& colliderShape = collider.shape.GetData();
//	if (!colliderShape.IsValid())
//	{
//		return;
//	}
//
//	if (color != currentDrawColor_)
//	{
//		SetRenderDrawColor(renderer, color);
//
//		currentDrawColor_ = color;
//	}
//
//	util::DrawB2ColliderShape(camera, renderer, colliderShape);
//}
//


