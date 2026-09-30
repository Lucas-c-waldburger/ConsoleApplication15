#pragma once
#include "ComponentConcepts.h"

struct ActiveState 
{
	constexpr bool operator==(const ActiveState&) const = default;
};
