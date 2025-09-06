#pragma once
#include "BaseComponent.h"
#include "../core/Bitset.h"


struct EntityFlags : BaseComponent<EntityFlags>
{
	ComponentBitset componentVisibilityFlags{ true };
	EventDataBitset eventProductionFlags{ true };
};