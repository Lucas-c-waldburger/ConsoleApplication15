#include "B2Shape.h"
#include "B2Chain.h"
#include "B2Common.h"

std::vector<B2ContactData> B2Shape::GetContactData() const
{
    return GetContactDataImpl(shapeHandle_, Handle<B2Shape>{});
}

std::vector<B2ContactData> 
B2Shape::GetContactDataWith(const Handle<B2Shape>& query) const
{
    if (!query.IsValid() || query == shapeHandle_)
    {
        return {};
    }

    return GetContactDataImpl(shapeHandle_, query);
}

B2Chain B2ChainSegmentShape::GetParentChain()
{
    if (!IsValid())
    {
        return {}; 
    }

    auto parentChainId = b2Shape_GetParentChain(GetHandle());

    return B2Chain(Handle<B2Chain>::Create(parentChainId));
}

std::pair<SDL_FPoint, SDL_FPoint> B2ChainSegmentShape::GetPoints() const
{
    if (!IsValid())
    {
        return {};
    }

    auto segShape = b2Shape_GetChainSegment(GetHandle());
    b2Transform tf = GetParentTransform();

    b2Vec2 worldPoint1 = b2TransformPoint(tf, segShape.segment.point1);
    b2Vec2 worldPoint2 = b2TransformPoint(tf, segShape.segment.point2);

    return std::make_pair(ToSDLFPointScaled(worldPoint1), 
                          ToSDLFPointScaled(worldPoint2));
}

