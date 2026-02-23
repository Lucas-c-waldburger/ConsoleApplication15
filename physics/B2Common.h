#pragma once
#include <cassert>
#include "B2Body.h"
#include "../core/Algorithms.h"

static B2ManifoldPoint ConvertManifoldPoint(const b2ManifoldPoint& point)
{
    return B2ManifoldPoint{
        .anchor = {
            .a = ToSDLFPointScaled(point.anchorA),
            .b = ToSDLFPointScaled(point.anchorB)
        },
        .normalImpulse = point.normalImpulse,
        .normalVelocity = point.normalVelocity,
        .persisted = point.persisted,
        .point = ToSDLFPointScaled(point.point),
        .separation = point.separation,
        .tangentImpulse = point.tangentImpulse,
        .totalNormalImpulse = point.totalNormalImpulse
    };
}

template <typename T, typename U>
    requires ((std::same_as<T, Handle<B2Shape>> || std::same_as<T, Handle<B2Body>>) &&
              (std::same_as<U, Handle<B2Shape>> || std::same_as<U, Handle<B2Body>>))
static std::vector<B2ContactData>
GetContactDataImpl(const T& srcHandle, const U& queryHandle)
{
    if (!srcHandle.IsValid())
    {
        return {};
    }

    B2Body queryBody{};
    if constexpr (std::same_as<U, Handle<B2Body>>)
    {
        queryBody = B2Body{ queryHandle };
    }

    b2ContactData contacts[16];
    int count = 0;

    if constexpr (std::same_as<T, Handle<B2Shape>>)
    {
        count = b2Shape_GetContactData(srcHandle, contacts, 16);
    }
    else
    {
        count = b2Body_GetContactData(srcHandle, contacts, 16);
    }

    if (count <= 0)
    {
        return {};
    }

    std::vector<B2ContactData> result;
    result.reserve(static_cast<size_t>(count));

    for (size_t i = 0; i < static_cast<size_t>(count); ++i)
    {
        auto& src = contacts[i];

        if (queryHandle.IsValid())
        {
            if constexpr (std::same_as<U, Handle<B2Shape>>)
            {
                if (!core::EqualsAny(queryHandle, src.shapeIdA, src.shapeIdB))
                {
                    continue;
                }
            }
            else // body
            {
                assert(queryBody.IsValid());

                if (!(queryBody.OwnsShape(src.shapeIdA) ||
                      queryBody.OwnsShape(src.shapeIdB)))
                {
                    continue;
                }
            }
        }

        auto& dest = result.emplace_back();

        dest.shapeHandleA = Handle<B2Shape>::Create(src.shapeIdA);
        dest.shapeHandleB = Handle<B2Shape>::Create(src.shapeIdB);

        assert(dest.shapeHandleA.IsValid());
        assert(dest.shapeHandleB.IsValid());

        dest.manifold.normal = ToSDLFPointScaled(src.manifold.normal);
        dest.manifold.rollingImpulse = src.manifold.rollingImpulse;

        if (src.manifold.pointCount > 0)
        {
            dest.manifold.points.first = ConvertManifoldPoint(src.manifold.points[0]);
        }
        if (src.manifold.pointCount > 1)
        {
            dest.manifold.points.second = ConvertManifoldPoint(src.manifold.points[1]);
        }
    }

    return result;
}
