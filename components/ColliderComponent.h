#pragma once
#include "BaseComponent.h"
#include "../physics/B2Shape.h"
#include "../core/ReadOnly.h"


//struct Collider : public BaseComponent<Collider, 8>
struct Collider : public BaseComponent<Collider>
{
    ReadOnly<B2Shape> shape;
};

