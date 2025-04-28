#pragma once
#include "BaseComponent.h"
#include "../physics/B2Shape.h"


struct Collider : public BaseComponent<Collider, 8>
{
    B2Shape shape;

    /*Handle<B2Shape> shapeHandle;

    float density = 1.0f;
    float friction = 0.5f;
    float restitution = 0.0f;
    bool isSensor = false;    */ 
};

