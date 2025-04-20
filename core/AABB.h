#pragma once
#include "commonObjects.h"
#include <SDL_rect.h>

struct AABB : public SDL_FRect
{
	constexpr AABB() : SDL_FRect{ 0.f, 0.f, 0.f, 0.f } {}
	constexpr AABB(float x_, float y_, float w_, float h_) :
		SDL_FRect{ x_, y_, w_, h_ } {}

	// TODO: move definitions to header, mark constexpr 
	bool Contains(const AABB& other) const;
	bool Intersects(const AABB& other) const;
	SDL_FPoint GetCenter() const;
	Dimensions<float> GetSizeHalf() const;

	constexpr bool operator==(const AABB& rhs) const {
		return x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h;
	}
};




