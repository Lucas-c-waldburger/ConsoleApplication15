#include "ColliderDrawSystem.h"
#include <ranges>

namespace test {

void ColliderDrawSystem::Update(float)
{
	auto entities = ECS::GetAllEntitiesWith<ShapeData>([](const auto& data) {
		return !data.points.empty();
	});

	std::ranges::sort(entities.begin(), entities.end(), [](const auto& e1, const auto& e2) {
		return e1.GetComponent<ShapeData>().ordinal >
		       e2.GetComponent<ShapeData>().ordinal;
	});

	auto& r = SDLite::Renderer();
	assert(r);

	SDL_Color origDrawColor = r.GetColor();
	SDL_Color currentDrawColor = origDrawColor;

	for (auto& e : entities)
	{
		auto& data = e.GetComponent<ShapeData>();

		if (data.color != currentDrawColor)
		{
			r.SetColor(data.color);
			currentDrawColor = data.color;
		}

		if (data.tempPoint.has_value())
		{
			data.points.emplace_back(*data.tempPoint);
		}

		if (data.points.size() == 1)
		{
			SDL_RenderDrawPointF(r, data.points[0].x, data.points[0].y);
		}
		else
		{
			switch (data.shapeType)
			{
			case B2Shape::Type::Polygon:
			{
				SDL_RenderDrawLinesF(r, data.points.data(), data.points.size());

				break;
			}
			case ShapeData::kUprightRect:
			{
				assert(data.points.size() == 2);

				auto p1 = data.points[0];
				auto p2 = data.points[1];

				auto rect = ResolvePointsToRect(p1, p2);

				SDL_RenderDrawRectF(r, &rect);

				break;
			}
			default:
			{
				break;
			}
			}
		}

		if (data.tempPoint.has_value())
		{
			data.points.pop_back();
		}
	}

	if (currentDrawColor != origDrawColor)
	{
		r.SetColor(origDrawColor);
	}
}



} // test

