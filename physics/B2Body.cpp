#include "B2Body.h"
#include "../core/HandleFactory.h"
#include "../core/TypeUtils.h"
#include "B2Common.h"
#include <cassert>
#include <ranges>

namespace {

template <typename T>
std::vector<T> GetShapesImpl(const Handle<B2Body>& bodyHandle)
{
    if (!bodyHandle.IsValid())
    {
        return {};
    }

    b2ShapeId shapeIds[B2Body::kMaxShapesPerBody];

    int count = b2Body_GetShapes(bodyHandle, shapeIds, B2Body::kMaxShapesPerBody);
    if (count <= 0)
    {
        return {};
    }

    std::vector<T> shapes;
    shapes.reserve(count);

    for (int i = 0; i < count; i++)
    {
        Handle<B2Shape> handle = Handle<B2Shape>::Create(shapeIds[i]);

        if (handle.IsValid())
        {
            B2Shape shape{ handle };
            shapes.emplace_back(std::move(shape));
        }
    }

    return shapes;
}


} // unnamed namespace

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

    Handle<B2Shape> shapeHandle = Handle<B2Shape>::Create(shapeId);

    assert(shapeHandle.IsValid());

    return B2Shape{ shapeHandle };
}

Result<B2Chain> B2Body::AddChain(const B2ChainDefinition& chainDef)
{
    if (GetBodyType() != Type::Static)
    {
        return MAKE_ERROR("Chains should only be added to static bodies");
    }

    if (chainDef.points.size() < 4)
    {
        return MAKE_ERROR("Chains must be made with at least 4 points");
    }

    auto points = chainDef.points | std::views::transform(ToB2VecScaled)
        | std::ranges::to<std::vector>();

    auto mats = chainDef.materials | std::views::transform([](const auto& mat) {
        return static_cast<b2SurfaceMaterial>(mat);
    }) | std::ranges::to<std::vector>();

    if (mats.empty())
    {
        mats.emplace_back(b2DefaultSurfaceMaterial());
    }
    if (mats.size() > 1 && mats.size() != points.size())
    {
        return MAKE_ERROR("Chain material count must either be 1 or "
            "equal to point count");
    }

    b2ChainDef def = b2DefaultChainDef();
    def.count = chainDef.points.size();
    def.enableSensorEvents = chainDef.enableSensorEvents;
    def.filter = {
        .categoryBits = chainDef.collisionFilter.categories,
        .maskBits = chainDef.collisionFilter.categoryMask,
        .groupIndex = chainDef.collisionFilter.groupIndex
    };
    def.isLoop = chainDef.isLoop;
    def.materialCount = mats.size();
    def.materials = mats.data();
    def.points = points.data();

    b2ChainId chainId = b2CreateChain(bodyHandle_, &def);
    if (!b2Chain_IsValid(chainId))
    {
        return MAKE_ERROR("Chain could not be created - returned null chain id");
    }

    return B2Chain{ Handle<B2Chain>::Create(chainId) };
}

std::vector<B2Shape> B2Body::GetShapes() 
{
    return GetShapesImpl<B2Shape>(bodyHandle_);
}

std::vector<ReadOnly<B2Shape>> B2Body::GetShapes() const
{
    return GetShapesImpl<ReadOnly<B2Shape>>(bodyHandle_);
}


std::unordered_set<Handle<B2Shape>> B2Body::GetShapeHandles() const
{
    auto shapes = GetShapes();

    std::unordered_set<Handle<B2Shape>> handles;
    std::transform(shapes.begin(), shapes.end(), std::inserter(handles, handles.end()), 
                   [](const auto& sh) { return sh.GetData().GetHandle(); });

    return handles;
}

bool B2Body::OwnsShape(b2ShapeId shapeId) const
{
    if (!b2Shape_IsValid(shapeId))
    {
        return false;
    }
    if (!IsValid())
    {
        return false;
    }

    return b2Shape_GetBody(shapeId) == bodyHandle_;
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

    return b2Shape_GetBody(shapeHandle) == bodyHandle_;
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

std::vector<B2ContactData> B2Body::GetContactData() const
{
    return GetContactDataImpl(bodyHandle_, Handle<B2Body>{});
}

std::vector<B2ContactData> 
B2Body::GetContactDataWith(const Handle<B2Body>& query) const
{
    return GetContactDataImpl(bodyHandle_, query);
}

std::vector<B2ContactData>
B2Body::GetContactDataWith(const Handle<B2Shape>& query) const
{
    return GetContactDataImpl(bodyHandle_, query);
}