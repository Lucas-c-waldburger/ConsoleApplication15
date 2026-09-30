#pragma once
#include "ComponentConcepts.h"


struct MarkedDestroyed 
{
	constexpr bool operator==(const MarkedDestroyed&) const = default;
};