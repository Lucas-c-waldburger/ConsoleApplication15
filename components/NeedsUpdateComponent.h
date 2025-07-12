#pragma once
#include "BaseComponent.h"

struct NeedsUpdate : public BaseComponent<NeedsUpdate>
{
	ComponentSignature components = 0;
};


