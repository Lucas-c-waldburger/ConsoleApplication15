#pragma once
#include "BaseComponent.h"
#include "../core/Handle.h"

struct Timer : BaseComponent<Timer>
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

	friend constexpr bool operator==(const Timer& lhs, const Timer& rhs)
	{
		return lhs.elapsed == rhs.elapsed && lhs.duration == rhs.duration &&
			   lhs.numRepeats == rhs.numRepeats && lhs.flags == rhs.flags;
	}
};