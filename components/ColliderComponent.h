#pragma once
#include "BaseComponent.h"
#include "../physics/B2Shape.h"
#include "../core/ReadOnly.h"

struct Collider : public BaseComponent<Collider>
{
    ReadOnly<B2Shape> shape;

    friend bool operator==(const Collider& lhs, const Collider& rhs) noexcept
    {
        return lhs.shape == rhs.shape;
    }
};

