#pragma once
#include "BaseComponent.h"
#include <SDL_rect.h>

struct CameraTarget : public BaseComponent<CameraTarget>
{
	SDL_FPoint offset = { 0.0f, 0.0f };
	float followSpeed = 5.0f;
	float stopRadius = 0.0f;

	friend constexpr bool operator==(const CameraTarget& lhs, const CameraTarget& rhs) noexcept
	{
		return lhs.offset.x == rhs.offset.x &&
			   lhs.offset.y == rhs.offset.y &&
			   lhs.followSpeed == rhs.followSpeed &&
			   lhs.stopRadius == rhs.stopRadius;
	}
};

