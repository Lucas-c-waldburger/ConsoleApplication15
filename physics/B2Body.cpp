#include "B2Body.h"
#include "../core/HandleFactory.h"
#include <cassert>

Result<Handle<B2Shape>> B2Body::AddShape(const B2ShapeDefinition& shapeDef)
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
    }
    case B2Shape::Type::Invalid: default:
        return MAKE_ERROR("Shape type was invalid");
    }

    Handle<B2Shape> shapeHandle = HandleFactory<B2Shape>::GetHandle(shapeId);

    assert(shapeHandle.IsValid());
    assert(shapeHandles_.insert(shapeHandle).second);

    return shapeHandle;
}
