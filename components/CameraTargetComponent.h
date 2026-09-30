#pragma once
#include "ComponentConcepts.h"
#include <SDL_rect.h>

struct CameraTarget
{
	SDL_FPoint offset = { 0.0f, 0.0f };
	float followSpeed = 5.0f;
	float stopRadius = 0.0f;

	constexpr bool operator==(const CameraTarget& rhs) const
	{
		return offset.x == rhs.offset.x &&
			   offset.y == rhs.offset.y &&
			   followSpeed == rhs.followSpeed &&
			   stopRadius == rhs.stopRadius;
	}
};

