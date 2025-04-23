#pragma once
#include <box2d/box2d.h>
#include <SDL.h>
#include <numbers>
#include <optional>
#include "core/Logger.h"
#include "core/commonObjects.h"
#include "core/Result.h"
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

using PointInPixels = SDL_FPoint;
using PointInMeters = b2Vec2;

static constexpr float kPixelsPerMeter = 30.0f;

inline constexpr float ToMeters(float pixels) { return pixels / kPixelsPerMeter; }
inline constexpr float ToPixels(float meters) { return meters * kPixelsPerMeter; }

inline constexpr SDL_FPoint ToSDLFPoint(const b2Vec2 vec)
{
    return { ToPixels(vec.x), ToPixels(vec.y) };
}
inline constexpr b2Vec2 ToB2Vec(const SDL_FPoint p)
{
    return { ToMeters(p.x), ToMeters(p.y) };
}

inline b2Rot AngleToB2Rot(float angleDeg)
{
    float angleRadians = angleDeg * (std::numbers::pi_v<float> / 180.0f);
    return b2MakeRot(angleRadians);
}

inline std::vector<b2Vec2> SDLFPointsToB2Vecs(const std::vector<SDL_FPoint>& points)
{
    std::vector<b2Vec2> vecPoints;
    vecPoints.reserve(points.size());

    std::transform(points.begin(), points.end(), std::back_inserter(vecPoints),
        [](const auto& p) { return ToB2Vec(p); });

    return vecPoints;
}

template <typename T>
struct B2IdEq
{
    bool operator==(const T& lhs, const T& rhs) const {
        return lhs.index1 == rhs.index1 &&
               lhs.world0 == rhs.world0 &&
               lhs.generation == rhs.generation;
    }
};

inline void HashCombine(std::size_t& seed, std::size_t value)
{
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename T>
struct B2IdHash
{
    size_t operator()(const T& id) const noexcept
    {
        std::size_t hash = 0;
        HashCombine(hash, std::hash<int32_t>{}(id.index1));
        HashCombine(hash, std::hash<uint16_t>{}(id.world0));
        HashCombine(hash, std::hash<uint16_t>{}(id.generation));

        return hash;
    }
};

template <typename T>
using B2IdUnorderedSet = std::unordered_set<T, B2IdHash<T>, B2IdEq<T>>;

// TODO: change b2 ids into our API Handles


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
        b2Vec2 convertedCenter = ToB2Vec(localCenter);
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
        b2Vec2 convertedPos = ToB2Vec(localPos);
        b2Rot convertedRot = AngleToB2Rot(localRotAngle);

        auto vecPoints = SDLFPointsToB2Vecs(points);

        b2Hull hull = b2ComputeHull(vecPoints.data(), vecPoints.size());

        return b2MakeOffsetPolygon(&hull, convertedPos, convertedRot);
    }

private:
    B2ShapeFactory() = default;
};

class Shape
{
public:
    friend class Body;

    enum class Type
    {
        Invalid = -1,
        Circle = b2_circleShape,
        Capsule = b2_capsuleShape,
        Segment = b2_segmentShape,
        Polygon = b2_polygonShape,
        ChainSegment = b2_chainSegmentShape
    };

    Shape() = default;

    bool IsValid() const 
    { 
        return b2Shape_IsValid(shapeId_) && b2Body_IsValid(b2Shape_GetBody(shapeId_));
    }

    void Destroy(bool updateBodyMass = true)
    {
        b2DestroyShape(shapeId_, updateBodyMass);
        shapeId_ = b2_nullShapeId;
    }

    float GetDensity() const
    {
        return b2Shape_GetDensity(shapeId_);
    }
    void SetDensity(float density, bool updateBody = true)
    {
        b2Shape_SetDensity(shapeId_, density, updateBody);
    }

    float GetFriction() const
    {
        return b2Shape_GetFriction(shapeId_);
    }
    void SetFriction(float friction)
    {
        b2Shape_SetFriction(shapeId_, friction);
    }

    float GetRestitution() const
    {
        return b2Shape_GetRestitution(shapeId_);
    }
    void SetRestitution(float restitution)
    {
        b2Shape_SetFriction(shapeId_, restitution);
    }

protected:
    Shape(b2ShapeId shapeId) : shapeId_(shapeId) {}

    b2BodyId GetBodyId() const
    {
        if (!b2Shape_IsValid(shapeId_))
        {
            return b2_nullBodyId;
        }

        return b2Shape_GetBody(shapeId_);
    }

    b2Transform GetParentTransform() const
    {
        return b2Body_GetTransform(GetBodyId());
    }

    b2ShapeId shapeId_ = b2_nullShapeId;
};

struct ShapeDefinition
{
    struct Data
    {
        std::optional<Dimensions<float>> dimensions;
        std::optional<std::vector<SDL_FPoint>> hull;
        std::optional<SDL_FPoint> localPosition;
        std::optional<float> localRotation;
        std::optional<float> radius;      
    };

    ShapeDefinition() : type(Shape::Type::Invalid), data(), def(b2DefaultShapeDef()) {}
    Shape::Type type;
    Data data;
    b2ShapeDef def;
};

template <typename T>
concept IsDerivedShape = std::derived_from<T, Shape> && 
                         std::constructible_from<T, b2ShapeId> &&
                         std::same_as<decltype(T::shapeType), Shape::Type>;

template <typename T> requires IsDerivedShape<T>
inline bool ShapeTypeMatches(b2ShapeId shapeId)
{
    Shape::Type castType = static_cast<Shape::Type>(b2Shape_GetType(shapeId));

    return castType == T::shapeType;
}

class PolygonShape : public Shape
{
public:
    static constexpr Shape::Type shapeType = Shape::Type::Polygon;

    PolygonShape() = default;
    PolygonShape(b2BodyId parentId, b2ShapeId shapeId) : Shape(shapeId) {}

    std::vector<SDL_FPoint> GetVertices()
    {
        if (!IsValid())
        {
            return {};
        }

        b2Polygon poly = b2Shape_GetPolygon(shapeId_);
        b2Transform tf = GetParentTransform();

        std::vector<SDL_FPoint> verts(poly.count + 1);
        for (int i = 0; i < poly.count; i++)
        {
            b2Vec2 worldPoint = b2TransformPoint(tf, poly.vertices[i]);

            verts[i] = ToSDLFPoint(worldPoint);
        }

        verts[poly.count] = verts[0];

        return verts;
    }
    
private:  
};

struct BodyDefinition
{
    BodyDefinition() : bodyData(b2DefaultBodyDef()) {}
    b2BodyDef bodyData;
    std::vector<ShapeDefinition> shapeDatas;
};

class Body
{
public:
    friend class B2World;
    static constexpr int kMaxShapesPerBody = 16;

    enum class Type
    {
        Static = b2_staticBody,
        Kinematic = b2_kinematicBody,
        Dynamic = b2_dynamicBody
    };

    Body() = default;
    explicit Body(b2BodyId bodyId) : bodyId_(bodyId) {}
     
    void Destroy()
    {
        b2DestroyBody(bodyId_);
        shapeIds_.clear();
        bodyId_ = b2_nullBodyId;
    }

    bool IsValid() const { return b2Body_IsValid(bodyId_); }

    Type GetType() const { return static_cast<Type>(b2Body_GetType(bodyId_)); }

    void SetFixedRotation(bool fixed) { b2Body_SetFixedRotation(bodyId_, fixed); }
    bool IsFixedRotation() const { return b2Body_IsFixedRotation(bodyId_); }

    void SetAwake(bool awake) { b2Body_SetAwake(bodyId_, awake); }
    bool IsAwake() const { return b2Body_IsAwake(bodyId_); }

    SDL_FPoint GetPosition() const { return ToSDLFPoint(b2Body_GetPosition(bodyId_)); }
    void SetPosition(SDL_FPoint newPos, bool wakeState = true)
    {
        b2Body_SetTransform(bodyId_, ToB2Vec(newPos), b2Body_GetRotation(bodyId_));
        SetAwake(wakeState);
    }

    float GetAngle() const { return b2Rot_GetAngle(b2Body_GetRotation(bodyId_)); }
    void SetAngle(float newAngle, bool wakeState = true)
    {
        b2Body_SetTransform(bodyId_, b2Body_GetPosition(bodyId_), AngleToB2Rot(newAngle));
        SetAwake(wakeState);
    }

    SDL_FPoint GetVelocity() const { return ToSDLFPoint(b2Body_GetLinearVelocity(bodyId_)); }

    void ApplyForce(SDL_FPoint force, std::optional<SDL_FPoint> pointOfContact = {})
    {
        if (!pointOfContact.has_value())
        {
            ApplyForceToCenter(force);
        }
        else
        {
            b2Body_ApplyForce(bodyId_, ToB2Vec(force), ToB2Vec(*pointOfContact), true);
        }
    }

    void ApplyForceToCenter(SDL_FPoint force) { b2Body_ApplyForceToCenter(bodyId_, ToB2Vec(force), true); }

    void ApplyLinearImpulse(SDL_FPoint impulse, std::optional<SDL_FPoint> pointOfContact = {})
    {
        if (!pointOfContact.has_value())
        {
            ApplyLinearImpulseToCenter(impulse);
        }
        else
        {
            b2Body_ApplyLinearImpulse(bodyId_, ToB2Vec(impulse), ToB2Vec(*pointOfContact), true);
        }
    }

    void ApplyLinearImpulseToCenter(SDL_FPoint impulse)
    { 
        b2Body_ApplyLinearImpulseToCenter(bodyId_, ToB2Vec(impulse), true);
    }

    // Shapes API
    int GetShapeCount() const { return b2Body_GetShapeCount(bodyId_); }

    template <typename T> requires IsDerivedShape<T>
    Result<T> GetShape(b2ShapeId shapeId)
    {
        if (!b2Body_IsValid(bodyId_))
        {
            return MAKE_ERROR("BodyId was invalid");
        }

        auto it = shapesIds_.find(shapeId);
        if (it == shapesIds_.end())
        {
            return MAKE_ERROR("ShapeId not found on body");
        }

        if (!b2Shape_IsValid(*it))
        {
            shapesIds_.erase(it);
            return MAKE_ERROR("ShapeId was invalid");
        }

        if (!ShapeTypeMatches<T>(*it))
        {
            return MAKE_ERROR("T::shapeType differs from held shape's type");
        }
        
        return T{ *it };
    }

    Result<b2ShapeId> AddShape(const ShapeDefinition& shapeDef)
    {
        if (GetShapeCount() >= kMaxShapesPerBody)
        {
            return MAKE_ERROR_FMT("Body cannot have more than {} shapes attached", kMaxShapesPerBody);
        }

        b2ShapeId shapeId = b2_nullShapeId;

        switch (shapeDef.type)
        {
        case Shape::Type::Polygon:
        {
            TRY(AddPolygon(bodyId_, shapeDef), id);
            shapeId = id;
        }
        case Shape::Type::Invalid: default:
            return MAKE_ERROR("Shape type was invalid");
        }

        assert(shapeIds_.insert(shapeId).second);

        return shapeId;
    }

    template <typename T> requires IsDerivedShape<T>
    Result<T> AddShape(const ShapeDefinition& shapeDef)
    {
        if (GetShapeCount() >= kMaxShapesPerBody)
        {
            return MAKE_ERROR_FMT("Body cannot have more than {} shapes attached", kMaxShapesPerBody);
        }

        b2ShapeId shapeId = b2_nullShapeId;

        if constexpr (std::same_as<T, PolygonShape>)
        {
            TRY(AddPolygon(bodyId_, shapeDef), id);
            shapeId = id;
        }

        if (!b2Shape_IsValid(shapeId))
        {
            return MAKE_ERROR("ShapeId was invalid");
        }

        assert(body.shapeIds_.insert(shapeId).second);

        return T{ shapeId };
    }

private:
    static Result<b2ShapeId> AddPolygon(b2BodyId bodyId, const ShapeDefinition& shapeDef)
    {
        if (shapeDef.data.dimensions.has_value())
        {
            return AddBox(bodyId, shapeDef);
        }
        
        return AddPolygonImpl(bodyId, shapeDef);
    }

    static Result<b2ShapeId> AddBox(b2BodyId bodyId, const ShapeDefinition& shapeDef)
    {
        auto& data = shapeDef.data;

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

        return b2CreatePolygonShape(bodyId, &shapeDef.def, &poly);
    }

    static Result<b2ShapeId> AddPolygonImpl(b2BodyId bodyId, const ShapeDefinition& shapeDef)
    {
        auto& data = shapeDef.data;

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

        return b2CreatePolygonShape(bodyId, &shapeDef.def, &poly);
    }

    b2BodyId bodyId_ = b2_nullBodyId;
    B2IdUnorderedSet<b2ShapeId> shapeIds_;
};


class B2World
{
public:
    static B2World Create(float gravX, float gravY)
    {
        B2World world;

        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = { gravX, gravY };

        world.worldId_ = b2CreateWorld(&worldDef);
        assert(b2World_IsValid(world.worldId_));

        return world;
    }

    bool IsValid() const { return b2World_IsValid(worldId_); }

    void Destroy()
    {
        if (b2World_IsValid(worldId_))
        {
            b2DestroyWorld(worldId_);
        }
    }

    SDL_FPoint GetGravity() const
    {
        b2Vec2 grav = b2World_GetGravity(worldId_);
        return { grav.x, grav.y };
    }
    void SetGravity(float newX, float newY)
    {
        b2Vec2 newGrav = { newX, newY };
        return b2World_SetGravity(worldId_, newGrav);
    }

    b2WorldId GetID() const { return worldId_; }

    Result<Body> AddBody(const BodyDefinition& bodyDef)
    {
        if (!IsValid())
        {
            return MAKE_ERROR("WorldId was invalid");
        }

        b2BodyId bodyId = b2CreateBody(worldId_, &bodyDef.bodyData);
        if (!b2Body_IsValid(bodyId))
        {
            return MAKE_ERROR("BodyId was invalid");
        }

        assert(!bodyIds_.contains(bodyId));

        Body body{ bodyId };

        for (const auto& shapeDef : bodyDef.shapeDatas)
        {
            TRY(body.AddShape(shapeDef));
        }

        bodyIds_.insert(bodyId);

        return body;
    }

    Result<Body> GetBody(b2BodyId bodyId)
    {
        if (!IsValid())
        {
            return MAKE_ERROR("WorldId was invalid");
        }
        if (!b2Body_IsValid(bodyId))
        {
            return MAKE_ERROR("BodyId was invalid");
        }
        if (!bodyIds_.contains(bodyId))
        {
            return MAKE_ERROR("BodyId does not belong to world");
        }

        Body body{ bodyId };

        b2ShapeId shapeIds[Body::kMaxShapesPerBody];
        int count = b2Body_GetShapes(bodyId, shapeIds, Body::kMaxShapesPerBody);
        body.shapeIds_.reserve(count);
        
        for (int i = 0; i < count; i++)
        {
            if (b2Shape_IsValid(shapeIds[i]))
            {
                body.shapeIds_.insert(shapeIds[i]);
            }
        }

        return body;
    }

private:
    B2World() = default;
    b2WorldId worldId_ = b2_nullWorldId;
    B2IdUnorderedSet<b2BodyId> bodyIds_;
};


static void RunBox2DSample()
{
    // setup

    b2WorldDef worldDef = b2DefaultWorldDef();
    
    worldDef.gravity = { 0.0f, -10.0f };

    b2WorldId worldId = b2CreateWorld(&worldDef);

    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = { 0.0f, -10.0f };

    b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);

    b2Polygon groundBox = b2MakeBox(50.0f, 10.0f);

    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = { 0.0f, 4.0f };
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.material.friction = 0.3f;

    b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

    // simulation

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;

    for (int i = 0; i < 90; ++i)
    {
        b2World_Step(worldId, timeStep, subStepCount);
        b2Vec2 position = b2Body_GetPosition(bodyId);
        b2Rot rotation = b2Body_GetRotation(bodyId);

        LOG_INFO_FMT("position = [{:.2f}, {:.2f}], rotation = {:.2f}",
            ToPixels(position.x), ToPixels(position.y), b2Rot_GetAngle(rotation));
    }

    b2DestroyWorld(worldId);
}


