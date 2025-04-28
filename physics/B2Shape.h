#pragma once
#include "B2Utils.h"
#include "B2Handle.h"
#include "../core/commonObjects.h"
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

    B2Shape() = default;

    Type GetShapeType() const { return static_cast<Type>(b2Shape_GetType(shapeHandle_)); }

    const Handle<B2Shape>& GetHandle() const { return shapeHandle_; }

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

    SDL_FRect GetBoundingBox() const
    {
        auto aabb = b2Shape_GetAABB(shapeHandle_);

        return B2AABBToSDLFRect(aabb);
    }

    template <typename T>
    T GetAs() const;

protected:
    explicit B2Shape(const Handle<B2Shape>& handle) : shapeHandle_(handle) {}

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

template <typename T>
concept SomeDerivedB2Shape = std::derived_from<T, B2Shape>&&
                             std::constructible_from<T, const Handle<B2Shape>&>&&
                             std::same_as<std::remove_cvref_t<decltype(T::shapeType)>, B2Shape::Type>;


struct B2ShapeDefinition
{
    struct Data
    {
        std::optional<Dimensions<float>> dimensions;
        std::optional<std::vector<SDL_FPoint>> hull;
        std::optional<SDL_FPoint> localPosition;
        std::optional<float> localRotation;
        std::optional<float> radius;
    };

    B2ShapeDefinition() : type(B2Shape::Type::Invalid), data(), def(b2DefaultShapeDef()) {}
    B2Shape::Type type;
    Data data;
    b2ShapeDef def;
};

template <typename T> requires SomeDerivedB2Shape<T>
inline bool ShapeTypeMatches(const Handle<B2Shape>& shapeHandle)
{
    B2Shape::Type castType = static_cast<B2Shape::Type>(b2Shape_GetType(shapeHandle));

    return castType == T::shapeType;
}

class B2PolygonShape : public B2Shape
{
public:
    static constexpr B2Shape::Type shapeType = B2Shape::Type::Polygon;

    B2PolygonShape() = default;
    explicit B2PolygonShape(const Handle<B2Shape>& handle) : B2Shape(handle) {}

    std::vector<SDL_FPoint> GetVertices()
    {
        if (!IsValid())
        {
            return {};
        }

        b2Polygon poly = b2Shape_GetPolygon(shapeHandle_);
        b2Transform tf = GetParentTransform();

        std::vector<SDL_FPoint> verts(poly.count + 1);
        for (int i = 0; i < poly.count; i++)
        {
            b2Vec2 worldPoint = b2TransformPoint(tf, poly.vertices[i]);

            verts[i] = ToSDLFPointScaled(worldPoint);
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
        if (!IsValid())
        {
            return std::numeric_limits<float>::min();
        }

        b2Circle circle = b2Shape_GetCircle(shapeHandle_);

        return ToPixels(circle.radius);
    }

    SDL_FPoint GetCenter() const
    {
        if (!IsValid())
        {
            return { std::numeric_limits<float>::min(),
                     std::numeric_limits<float>::min() };
        }

        b2Circle circle = b2Shape_GetCircle(shapeHandle_);
        b2Transform tf = GetParentTransform();

        b2Vec2 worldPoint = b2TransformPoint(tf, circle.center);

        return ToSDLFPointScaled(worldPoint);
    }

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
