#pragma once
#include "BaseComponent.h"
#include "../core/Handle.h"

struct Timer : BaseComponent<Timer>
{
	enum Flag : uint8_t
	{
		Active = 1 << 0,
		Repeating = 1 << 1,
		RemoveOnExpiry = 1 << 2
	};

	float time = 0.0f;
	float duration = 0.0f;
	uint8_t flags = 0;
};