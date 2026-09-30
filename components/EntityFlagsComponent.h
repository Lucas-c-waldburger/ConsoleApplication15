#pragma once
#include "ComponentConcepts.h"
#include "../core/Bitset.h"


struct EntityFlags
{
	ComponentBitset componentVisibilityFlags{ true };
	EventDataBitset eventProductionFlags{ true };
};