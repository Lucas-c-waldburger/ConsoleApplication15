#include "AABB.h"

bool AABB::Contains(const AABB& other) const
{
	return (other.x >= x && other.x + other.w <= x + w) &&
		(other.y >= y && other.y + other.h <= y + h);
}

bool AABB::Intersects(const AABB& other) const
{
	return !(other.x + other.w < x || other.x > x + w ||
		other.y + other.h < y || other.y > y + h);
}

SDL_FPoint AABB::GetCenter() const
{
	return { x + (w / 2.0f), y + (h / 2.0f) };
}

Dimensions<float> AABB::GetSizeHalf() const
{
	return { w / 2.0f, h / 2.0f };
}
