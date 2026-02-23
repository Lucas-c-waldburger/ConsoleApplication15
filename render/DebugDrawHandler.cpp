#include "DebugDrawHandler.h"
#include "../camera/Camera.h"
#include "../components/ColliderComponent.h"
#include "../components/RenderableComponent.h"
#include "../physics/B2Chain.h"
#include <ranges>

namespace {

void AddCirclePerimeterPoints(SDL_FPoint center, float radius,
							  std::vector<SDL_FPoint>& points)
{
	size_t firstCirclePointIdx = points.size();

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

	points.push_back(points[firstCirclePointIdx]);
}

void AddRectPoints(SDL_Rect rect, std::vector<SDL_FPoint>& points)
{
	SDL_FPoint firstPoint{ static_cast<float>(rect.x), static_cast<float>(rect.y) };
	points.push_back(firstPoint);
	points.push_back({ static_cast<float>(rect.x + rect.w), static_cast<float>(rect.y) });
	points.push_back({ static_cast<float>(rect.x + rect.w), static_cast<float>(rect.y + rect.h) });
	points.push_back({ static_cast<float>(rect.x), static_cast<float>(rect.y + rect.h) });
	points.push_back(firstPoint);
}

void RotateRectPoints(std::vector<SDL_FPoint>& points, SDL_FPoint center, float angleDegrees)
{
	assert(points.size() >= 5);

	float radians = angleDegrees * (M_PI / 180.0f);
	float cosA = std::cos(radians);
	float sinA = std::sin(radians);

	int start = points.size() - 5;
	for (int i = start; i < static_cast<int>(points.size()) - 1; ++i)
	{
		float dx = points[i].x - center.x;
		float dy = points[i].y - center.y;

		float x = dx * cosA - dy * sinA;
		float y = dx * sinA + dy * cosA;

		points[i].x = x + center.x;
		points[i].y = y + center.y;
	}

	points.back() = points[start];
}



} // unnamed

void DebugDrawHandler::Clear()
{
	debugDrawPoints_.clear();
	shapes_.clear();
}

void DebugDrawHandler::Reserve(size_t size) 
{ 
	debugDrawPoints_.reserve(size); 
}

void DebugDrawHandler::Reset(size_t newSize) 
{ 
	Clear(); 
	Reserve(newSize); 
}

void DebugDrawHandler::AddBoundingBox(SDL_Rect renderRect, float rotation,
									  const RenderProfile& profile)
{
	if (!shapes_.empty())
	{
		// Close the previous shape
		shapes_.back().shapeEndIndex = debugDrawPoints_.size();
	}

	shapes_.emplace_back(profile.debugDraw.boundingBox.color, 0);

	AddRectPoints(renderRect, debugDrawPoints_); // renderRect already in screen space

	if (rotation != 0.0f)
	{
		SDL_FPoint rectCenter{
			static_cast<float>(renderRect.x) + (static_cast<float>(renderRect.w) / 2.0f),
			static_cast<float>(renderRect.y) + (static_cast<float>(renderRect.h) / 2.0f)
		};

		RotateRectPoints(debugDrawPoints_, rectCenter, rotation);
	}
}

void DebugDrawHandler::AddColliderShape(const Camera& camera, const Collider& collider,
										const RenderProfile& profile)
{
	const auto& shapeData = collider.shape.GetData();
	if (!shapeData.IsValid())
	{
		return;
	}

	if (!camera.GetViewport().IntersectsBoundingBox(shapeData.GetBoundingBox()))
	{
		return;
	}

	if (!shapes_.empty())
	{
		// Close the previous shape
		shapes_.back().shapeEndIndex = debugDrawPoints_.size();
	}

	shapes_.emplace_back(profile.debugDraw.collider.color, 0);

	auto toScreen = [&camera](const auto& p) { 
		return camera.WorldToScreen<SDL_FPoint>(p); 
	};
	const size_t sizeBeforeAdding = debugDrawPoints_.size();

	switch (shapeData.GetShapeType())
	{
	case B2Shape::Type::Polygon:
	{
		auto polyShape = shapeData.GetAs<B2PolygonShape>();

		auto verts = polyShape.GetVertices();

		std::transform(verts.begin(), verts.end(), verts.begin(), toScreen);

		debugDrawPoints_.insert(debugDrawPoints_.end(),
			std::make_move_iterator(verts.begin()),
			std::make_move_iterator(verts.end()));

		break;
	}
	case B2Shape::Type::Circle:
	{
		auto circleShape = shapeData.GetAs<B2CircleShape>();

		SDL_FPoint center = circleShape.GetCenter();
		float radius = circleShape.GetRadius();

		AddCirclePerimeterPoints(center, radius, debugDrawPoints_);

		std::transform(debugDrawPoints_.begin() + sizeBeforeAdding,
			           debugDrawPoints_.end(),
			           debugDrawPoints_.begin() + sizeBeforeAdding, toScreen);

		break;
	}
	case B2Shape::Type::ChainSegment:
	{
		auto chainSegmentShape = shapeData.GetAs<B2ChainSegmentShape>();
		auto chain = chainSegmentShape.GetParentChain();
		if (!chain.IsValid())
		{
			break;
		}
		if (handledChains_.contains(chain.GetHandle()))
		{
			break;
		}
		handledChains_.insert(chain.GetHandle());

		auto segments = chain.GetSegments();

		auto segPoints = segments
		| std::views::filter([](const auto& seg) { return seg.IsValid(); })
		| std::views::transform([](const auto& seg) { return seg.GetPoints().first; }) 
		| std::views::transform(toScreen) | std::ranges::to<std::vector>();

		if (segPoints.empty())
		{
			break;
		}

		segPoints.emplace_back(toScreen(segments.back().GetPoints().second));

		debugDrawPoints_.insert(debugDrawPoints_.end(),
			std::make_move_iterator(segPoints.begin()),
			std::make_move_iterator(segPoints.end()));

		break;
	}

	case B2Shape::Type::Invalid: default:
		LOG_ERROR("Unsupported B2ShapeType");
	}
}

void DebugDrawHandler::Draw(SDL_Renderer* renderer)
{
	handledChains_.clear();

	if (debugDrawPoints_.empty() || shapes_.empty())
	{
		return;
	}

	// Close the last shape
	shapes_.back().shapeEndIndex = debugDrawPoints_.size();

	SDL_Color originalDrawColor = GetRenderDrawColor(renderer);
	SDL_Color currentDrawColor = originalDrawColor;

	size_t currentPointIdx = 0;
	for (auto& [color, shapeEndIdx] : shapes_)
	{
		if (color != currentDrawColor)
		{
			SetRenderDrawColor(renderer, color);
			currentDrawColor = color;
		}

		SDL_RenderDrawLinesF(renderer, debugDrawPoints_.data() + currentPointIdx,
							 static_cast<int>(shapeEndIdx - currentPointIdx));

		currentPointIdx = shapeEndIdx;
	}

	if (currentDrawColor != originalDrawColor)
	{
		SetRenderDrawColor(renderer, originalDrawColor);
	}
}