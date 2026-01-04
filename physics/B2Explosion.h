#pragma once
#include <box2d/box2d.h>
#include <SDL_rect.h>

struct B2ExplosionDefinition
{
	uint64_t categoryBitMask = 0xFFFFFFFFFFFFFFFFULL;
	SDL_FPoint position = { 0.0f, 0.0f };
	float radius = 0.0f;
	float falloff = 0.0f;
	float impulsePerLength = 0.0f;
};