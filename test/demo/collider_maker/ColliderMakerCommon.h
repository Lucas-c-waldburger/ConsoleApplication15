#pragma once
#include "../../Fixtures.h"

namespace test {

enum class MenuType
{
    Unknown,
    Main,
    Layer,
    Collider,
    Sprite
};

struct ShapeData
{
	static constexpr B2Shape::Type kUprightRect = static_cast<B2Shape::Type>(5);

	B2Shape::Type shapeType;
	std::vector<SDL_FPoint> points;
	std::optional<SDL_FPoint> tempPoint;
	SDL_Color color;
	size_t ordinal = 0;
};

struct MouseOverPoint {};

inline SDL_Color GetRandColor()
{
	return SDL_Color{
		.r = static_cast<uint8_t>(rand() % 256),
		.g = static_cast<uint8_t>(rand() % 256),
		.b = static_cast<uint8_t>(rand() % 256),
		.a = 255
	};
}

inline SDL_FRect ResolvePointsToRect(SDL_FPoint p1, SDL_FPoint p2)
{
	return 	{
		.x = std::min(p1.x, p2.x),
		.y = std::min(p1.y, p2.y),
		.w = std::fabs(p2.x - p1.x),
		.h = std::fabs(p2.y - p1.y)
	};
}

inline SDL_FPoint ComputeCentroid(const std::vector<SDL_FPoint>& pts)
{
    float area = 0.0f;
    float cx = 0.0f;
    float cy = 0.0f;

    int count = (int)pts.size();

    for (int i = 0; i < count; ++i)
    {
        const SDL_FPoint& p0 = pts[i];
        const SDL_FPoint& p1 = pts[(i + 1) % count];

        float cross = p0.x * p1.y - p1.x * p0.y;

        area += cross;
        cx += (p0.x + p1.x) * cross;
        cy += (p0.y + p1.y) * cross;
    }

    area *= 0.5f;

    if (std::fabs(area) < 1e-6f)
    {
        return { 0.0f, 0.0f }; // degenerate
    }

    float inv = 1.0f / (6.0f * area);

    return {
        cx * inv,
        cy * inv
    };
}
} // test