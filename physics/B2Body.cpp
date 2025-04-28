#include "B2Body.h"
#include "../core/HandleFactory.h"
#include <cassert>

Result<B2Shape> B2Body::GetShape(const Handle<B2Shape>& shapeHandle)
{
    if (!IsValid())
    {
        return MAKE_ERROR("BodyId was invalid");
    }
    if (!shapeHandle.IsValid())
    {
        return MAKE_ERROR("ShapeId was invalid");
    }
    if (!OwnsShape(shapeHandle))
    {
        return MAKE_ERROR("ShapeId not found on body");
    }

    return B2Shape{ shapeHandle };
}

Result<B2Shape> B2Body::AddShape(const B2ShapeDefinition& shapeDef)
{
    if (GetShapeCount() >= kMaxShapesPerBody)
    {
        return MAKE_ERROR_FMT("Body cannot have more than {} shapes attached", kMaxShapesPerBody);
    }

    b2ShapeId shapeId = b2_nullShapeId;

    switch (shapeDef.type)
    {
    case B2Shape::Type::Polygon:
    {
        TRY(AddPolygon(bodyHandle_, shapeDef), id);
        shapeId = id;

        break;
    }
    case B2Shape::Type::Circle:
    {
        TRY(AddCircle(bodyHandle_, shapeDef), id);
        shapeId = id;

        break;
    }
    case B2Shape::Type::Invalid: default:
        return MAKE_ERROR("Shape type was invalid or unsupported");
    }

    Handle<B2Shape> shapeHandle = HandleFactory<B2Shape>::GetHandle(shapeId);

    assert(shapeHandle.IsValid());

    return B2Shape{ shapeHandle };
}

std::unordered_set<Handle<B2Shape>> B2Body::GetShapeHandles() const
{
    if (!IsValid())
    {
        return {};
    }

    b2ShapeId shapeIds[B2Body::kMaxShapesPerBody];

    int count = b2Body_GetShapes(bodyHandle_, shapeIds, B2Body::kMaxShapesPerBody);
    if (count <= 0)
    {
        return {};
    }

    std::unordered_set<Handle<B2Shape>> shapeHandles;
    shapeHandles.reserve(count);

    for (int i = 0; i < count; i++)
    {
        Handle<B2Shape> shapeHandle = HandleFactory<B2Shape>::GetHandle(shapeIds[i]);

        if (shapeHandle.IsValid())
        {
            shapeHandles.insert(shapeHandle);
        }
    }

    return shapeHandles;
}

bool B2Body::OwnsShape(const Handle<B2Shape>& shapeHandle) const
{
    if (!shapeHandle.IsValid())
    {
        return false;
    }
    if (!this->IsValid())
    {
        return false;
    }

    b2ShapeId shapeIds[B2Body::kMaxShapesPerBody];
    int count = b2Body_GetShapes(bodyHandle_, shapeIds, B2Body::kMaxShapesPerBody);

    for (int i = 0; i < count; i++)
    {
        if (shapeHandle == shapeIds[i])
        {
            return true;
        }
    }

    return false;
}