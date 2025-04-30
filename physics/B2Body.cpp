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

    switch (shapeDef.shapeParams.shapeType)
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

Result<b2ShapeId> B2Body::AddCircle(b2BodyId bodyId, const B2ShapeDefinition& shapeDef)
{
    auto& data = shapeDef.shapeParams;

    if (!data.radius.has_value())
    {
        return MAKE_ERROR("shape type was circle but radius had no value");
    }

    b2Circle circle = B2ShapeFactory::MakeCircle(
        data.localPosition.value_or(SDL_FPoint{ 0.0, 0.0 }), *data.radius);

    return b2CreateCircleShape(bodyId, &shapeDef.shapeDef, &circle);
}

Result<b2ShapeId> B2Body::AddPolygon(b2BodyId bodyId, const B2ShapeDefinition& shapeDef)
{
    if (shapeDef.shapeParams.dimensions.has_value())
    {
        return AddBox(bodyId, shapeDef);
    }

    return AddPolygonImpl(bodyId, shapeDef);
}

// TODO: move to unnamed namespace up top and out of class decl
Result<b2ShapeId> B2Body::AddPolygonImpl(b2BodyId bodyId, const B2ShapeDefinition& shapeDef)
{
    auto& data = shapeDef.shapeParams;

    if (!data.hull.has_value())
    {
        return MAKE_ERROR("shape type was polygon but hull had no value");
    }

    b2Polygon poly;

    if (data.localPosition.has_value() || data.localRotation.has_value())
    {
        poly = B2ShapeFactory::MakeOffsetPolygon(*data.hull,
            data.localPosition.value_or(SDL_FPoint{ 0.0f, 0.0f }),
            data.localRotation.value_or(0.0f)
        );
    }
    else if (data.radius.has_value())
    {
        poly = B2ShapeFactory::MakePolygon(*data.hull, *data.radius);
    }
    else
    {
        return MAKE_ERROR("shape type was polygon but had no radius OR had no offset data");
    }

    return b2CreatePolygonShape(bodyId, &shapeDef.shapeDef, &poly);
}

Result<b2ShapeId> B2Body::AddBox(b2BodyId bodyId, const B2ShapeDefinition& shapeDef)
{
    auto& data = shapeDef.shapeParams;

    assert(data.dimensions.has_value());

    b2Polygon poly;

    if (data.localPosition.has_value() || data.localRotation.has_value())
    {
        poly = B2ShapeFactory::MakeOffsetBox(*data.dimensions,
            data.localPosition.value_or(SDL_FPoint{ 0.0f, 0.0f }),
            data.localRotation.value_or(0.0f)
        );
    }
    else
    {
        poly = B2ShapeFactory::MakeBox(*data.dimensions);
    }

    return b2CreatePolygonShape(bodyId, &shapeDef.shapeDef, &poly);
}
