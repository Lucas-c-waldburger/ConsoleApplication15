#pragma once
#include "../core/SizedEnum.h"

enum class Phase
{
	Invalid = -1,
	Setup = 0,
	Input,
	Intent,
	Simulation,
	SimResponse,
	Presentation,
	Cleanup,
	ENUM_SIZE_
};
static_assert(SomeSizedEnum<Phase>);