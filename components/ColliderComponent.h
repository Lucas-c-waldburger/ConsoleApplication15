#pragma once
#include "BaseComponent.h"
#include "../physics/B2Shape.h"


struct Collider : public BaseComponent<Collider, 8>
{
    B2Shape shape;
};

