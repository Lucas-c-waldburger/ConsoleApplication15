#pragma once
#include "ComponentConcepts.h"
#include "../physics/B2Shape.h"
#include "../core/ReadOnly.h"

struct Collider
{
    ReadOnly<B2Shape> shape;

    bool operator==(const Collider&) const = default;
};

