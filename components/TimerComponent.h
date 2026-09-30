#pragma once
#include "ComponentConcepts.h"
#include "../core/Handle.h"

struct Timer
{
	enum Flag : uint8_t
	{
		Active = 1 << 0,
		RemoveOnExpiry = 1 << 1
	};

	float elapsed = 0.0f;
	float duration = 0.0f;
	int numRepeats = 0;
	uint8_t flags = 0;

	constexpr bool operator==(const Timer&) const = default;
};