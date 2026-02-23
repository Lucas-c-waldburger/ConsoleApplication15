#pragma once
#include "B2Handle.h"
#include <SDL_rect.h>
#include <optional>

struct B2ManifoldPoint
{
	struct Anchor
	{
		SDL_FPoint a = { 0.0f, 0.0f };
		SDL_FPoint b = { 0.0f, 0.0f };
	};

	Anchor anchor;
	float normalImpulse = 0.0f;
	float normalVelocity = 0.0f;
	bool persisted = false;
	SDL_FPoint point = { 0.0f, 0.0f };
	float separation = 0.0f;
	float tangentImpulse = 0.0f;
	float totalNormalImpulse = 0.0f;
};

using B2ManifoldPoints = std::pair<std::optional<B2ManifoldPoint>, 
								   std::optional<B2ManifoldPoint>>;

struct B2Manifold
{
	SDL_FPoint normal = { 0.0f, 0.0f };
	B2ManifoldPoints points;
	float rollingImpulse = 0.0f;
};

struct B2ContactData
{
	B2Manifold manifold;
	Handle<B2Shape> shapeHandleA;
	Handle<B2Shape> shapeHandleB;
};