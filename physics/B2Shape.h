#pragma once
#include "B2Utils.h"
#include "B2Handle.h"
#include "B2ContactData.h"
#include "../core/commonObjects.h"
#include "B2CollisionFilter.h"
#include <optional>
#include <SDL.h>
#include <numeric>

class B2ShapeFactory
{
public:
    static b2Polygon MakeBox(Dimensions<float> dimensions)
    {
        float halfW = ToMeters(dimensions.w / 2.0f);
        float halfH = ToMeters(dimensions.h / 2.0f);

        return b2MakeBox(halfW, halfH);
    }
    static b2Polygon MakeOffsetBox(Dimensions<float> dimensions, SDL_FPoint localCenter,
                                   float localRotAngle)
    {
        b2Vec2 convertedCenter = ToB2VecScaled(localCenter);
        b2Rot convertedRot = AngleToB2Rot(localRotAngle);

        float halfW = ToMeters(dimensions.w / 2.0f);
        float halfH = ToMeters(dimensions.h / 2.0f);

        return b2MakeOffsetBox(halfW, halfH, convertedCenter, convertedRot);
    }

    static b2Polygon MakePolygon(const std::vector<SDL_FPoint>& points, float radius)
    {
        auto vecPoints = SDLFPointsToB2Vecs(points);

        b2Hull hull = b2ComputeHull(vecPoints.data(), vecPoints.size());

        return b2MakePolygon(&hull, radius);
    }
    static b2Polygon MakeOffsetPolygon(const std::vector<SDL_FPoint>& points,
                                       SDL_FPoint localPos, float localRotAngle)
    {
        b2Vec2 convertedPos = ToB2VecScaled(localPos);
        b2Rot convertedRot = AngleToB2Rot(localRotAngle);

        auto vecPoints = SDLFPointsToB2Vecs(points);

        b2Hull hull = b2ComputeHull(vecPoints.data(), vecPoints.size());

        return b2MakeOffsetPolygon(&hull, convertedPos, convertedRot);
    }

    static b2Circle MakeCircle(SDL_FPoint center, float radius)
    {
        return b2Circle{ .center = ToB2VecScaled(center), .radius = ToMeters(radius) };
    }

private:
    B2ShapeFactory() = default;
};

class B2Shape
{
public:
    friend class B2Body;

    enum class Type
    {
        Invalid = -1,
        Circle = b2_circleShape,
        Capsule = b2_capsuleShape,
        Segment = b2_segmentShape,
        Polygon = b2_polygonShape,
        ChainSegment = b2_chainSegmentShape
    };

    enum CoordinateSpace
    {
        WorldSpace = 1,
        LocalSpace
    };

    struct EventsEnabled
    {
        bool contact = false;
        bool sensor = false;
        bool hit = false;
    };

    B2Shape() = default;
    explicit B2Shape(const Handle<B2Shape>& handle) : shapeHandle_(handle) {}

    bool operator==(const B2Shape& rhs) const { return shapeHandle_ == rhs.shapeHandle_; }

    Type GetShapeType() const { return static_cast<Type>(b2Shape_GetType(shapeHandle_)); }

    const Handle<B2Shape>& GetHandle() const { return shapeHandle_; }

    Handle<B2Body> GetParentBodyHandle() const 
    {
        if (!IsValid())
        {
            return {};
        }

        return Handle<B2Body>::Create(b2Shape_GetBody(shapeHandle_));
    }

    bool IsValid() const
    {
        return shapeHandle_.IsValid() && b2Body_IsValid(b2Shape_GetBody(shapeHandle_));
    }

    void Destroy(bool updateBodyMass = true)
    {
        b2DestroyShape(shapeHandle_, updateBodyMass);
        shapeHandle_ = Handle<B2Shape>{};
    }

    float GetDensity() const
    {
        return b2Shape_GetDensity(shapeHandle_);
    }
    void SetDensity(float density, bool updateBody = true)
    {
        b2Shape_SetDensity(shapeHandle_, density, updateBody);
    }

    float GetFriction() const
    {
        return b2Shape_GetFriction(shapeHandle_);
    }
    void SetFriction(float friction)
    {
        b2Shape_SetFriction(shapeHandle_, friction);
    }

    float GetRestitution() const
    {
        return b2Shape_GetRestitution(shapeHandle_);
    }
    void SetRestitution(float restitution)
    {
        b2Shape_SetFriction(shapeHandle_, restitution);
    }

    B2CollisionFilter GetCollisionFilter() const
    {
        auto filter = b2Shape_GetFilter(shapeHandle_);

        return B2CollisionFilter{
            .categories = filter.categoryBits,
            .categoryMask = filter.maskBits,
            .groupIndex = filter.groupIndex
        };
    }
    void SetCollisionFilter(const B2CollisionFilter& pubFilter)
    {
        b2Filter filter{
            .categoryBits = pubFilter.categories,
            .maskBits = pubFilter.categoryMask,
            .groupIndex = pubFilter.groupIndex
        };

        b2Shape_SetFilter(shapeHandle_, filter);
    }

    EventsEnabled GetEventsEnabled() const
    {
        return EventsEnabled{
            .contact = b2Shape_AreContactEventsEnabled(shapeHandle_),
            .sensor = b2Shape_AreSensorEventsEnabled(shapeHandle_),
            .hit = b2Shape_AreHitEventsEnabled(shapeHandle_)
        };
    }
    void EnableEvents(const EventsEnabled& enable)
    {
        b2Shape_EnableContactEvents(shapeHandle_, enable.contact);
        b2Shape_EnableSensorEvents(shapeHandle_, enable.sensor);
        b2Shape_EnableHitEvents(shapeHandle_, enable.hit);
    }

    bool IsCollisionEnabled() const
    {
        auto filter = b2Shape_GetFilter(shapeHandle_);

        return filter.categoryBits != 0 && filter.maskBits != 0;
    }

    bool IsSensor() const
    {
        return b2Shape_IsSensor(shapeHandle_);
    }

    SDL_FRect GetBoundingBox() const
    {
        auto aabb = b2Shape_GetAABB(shapeHandle_);

        return B2AABBToSDLFRect(aabb);
    }

    bool IsPointInside(SDL_FPoint point) const
    {
        if (!IsValid()) { return false; }

        return b2Shape_TestPoint(shapeHandle_, ToB2VecScaled(point));
    }



    std::vector<B2ContactData> GetContactData() const;
    std::vector<B2ContactData> GetContactDataWith(const Handle<B2Shape>& query) const;

    std::vector<Handle<B2Shape>> GetSensorOverlaps() const;

    template <typename T>
    T GetAs() const;

protected:
    b2BodyId GetBodyId() const
    {
        if (!shapeHandle_.IsValid())
        {
            return b2_nullBodyId;
        }

        return b2Shape_GetBody(shapeHandle_);
    }

    b2Transform GetParentTransform() const
    {
        return b2Body_GetTransform(GetBodyId());
    }

    Handle<B2Shape> shapeHandle_;
};

inline constexpr std::string_view ToString(B2Shape::Type type)
{
    switch (type)
    {
    case B2Shape::Type::Circle: return "Circle";
    case B2Shape::Type::Capsule: return "Capsule";
    case B2Shape::Type::Segment: return "Segment";
    case B2Shape::Type::Polygon: return "Polygon";
    case B2Shape::Type::ChainSegment: return "ChainSegment";
    }

    return "<unknown>";
}

namespace std {
template <>
struct hash<B2Shape> {
    size_t operator()(const B2Shape& shape) const noexcept {
        return std::hash<Handle<B2Shape>>{}(shape.GetHandle());
    };
};
}

template <typename T>
concept SomeDerivedB2Shape = std::derived_from<T, B2Shape>&&
                             std::constructible_from<T, const Handle<B2Shape>&>&&
                             std::same_as<std::remove_cvref_t<decltype(T::shapeType)>, B2Shape::Type>;


struct B2ShapeParameters
{
    B2Shape::Type shapeType = B2Shape::Type::Invalid;

    std::optional<Dimensions<float>> dimensions;
    std::optional<std::vector<SDL_FPoint>> hull;
    std::optional<SDL_FPoint> localPosition;
    std::optional<float> localRotation;
    std::optional<float> radius;
};

struct B2ShapeDefinition
{
    B2ShapeDefinition() : shapeParams(), shapeDef(b2DefaultShapeDef()) {}
    B2ShapeParameters shapeParams;
    b2ShapeDef shapeDef;
};

template <typename T> requires SomeDerivedB2Shape<T>
inline bool ShapeTypeMatches(const Handle<B2Shape>& shapeHandle)
{
    return T::shapeType == static_cast<B2Shape::Type>(b2Shape_GetType(shapeHandle));
}

class B2PolygonShape : public B2Shape
{
public:
    static constexpr B2Shape::Type shapeType = B2Shape::Type::Polygon;

    B2PolygonShape() = default;
    explicit B2PolygonShape(const Handle<B2Shape>& handle) : B2Shape(handle) {}

    size_t GetVertexCount() const
    {
        return (IsValid()) ? b2Shape_GetPolygon(shapeHandle_).count : 0;
    }

    float GetRadius() const
    {
        return (IsValid()) 
            ? ToPixels(b2Shape_GetPolygon(shapeHandle_).radius) 
            : std::numeric_limits<float>::min();
    }

    std::vector<SDL_FPoint> GetVertices(B2Shape::CoordinateSpace space = WorldSpace) const
    {
        if (!IsValid())
        {
            return {};
        }

        b2Polygon poly = b2Shape_GetPolygon(shapeHandle_);
        b2Transform tf{};
        if (space == WorldSpace)
        {
            tf = GetParentTransform();
		}

        std::vector<SDL_FPoint> verts(poly.count + 1);
        for (int i = 0; i < poly.count; i++)
        {
            b2Vec2 worldPoint = b2TransformPoint(tf, poly.vertices[i]);

            verts[i] = ToSDLFPointScaled(space == WorldSpace 
                ? b2TransformPoint(tf, poly.vertices[i]) 
                : poly.vertices[i]);
        }

        verts[poly.count] = verts[0];

        return verts;
    }

private:
};

class B2CircleShape : public B2Shape
{
public:
    static constexpr B2Shape::Type shapeType = B2Shape::Type::Circle;

    B2CircleShape() = default;
    explicit B2CircleShape(const Handle<B2Shape>& handle) : B2Shape(handle) {}

    float GetRadius() const
    {
        return (IsValid())
            ? ToPixels(b2Shape_GetCircle(shapeHandle_).radius)
            : std::numeric_limits<float>::min();
    }

    SDL_FPoint GetCenter(B2Shape::CoordinateSpace space = WorldSpace) const
    {
        if (!IsValid())
        {
            return { std::numeric_limits<float>::min(),
                     std::numeric_limits<float>::min() };
        }

        b2Circle circle = b2Shape_GetCircle(shapeHandle_);

        if (space == LocalSpace)
        {
            return ToSDLFPointScaled(circle.center);
		}

        b2Transform tf = GetParentTransform();

        return ToSDLFPointScaled(b2TransformPoint(tf, circle.center));
    }

private:
};

class B2Chain;

class B2ChainSegmentShape : public B2Shape
{
public:
    static constexpr B2Shape::Type shapeType = B2Shape::Type::ChainSegment;

    B2ChainSegmentShape() = default;
    explicit B2ChainSegmentShape(const Handle<B2Shape>& handle) : B2Shape(handle) {}

    B2Chain GetParentChain();

    std::pair<SDL_FPoint, SDL_FPoint> GetPoints(B2Shape::CoordinateSpace space = WorldSpace) const;

private:
};

template <typename T> 
inline T B2Shape::GetAs() const
{
    static_assert(SomeDerivedB2Shape<T>);

    if (ShapeTypeMatches<T>(shapeHandle_))
    {
        return T{ shapeHandle_ };
    }

    return T{};
}