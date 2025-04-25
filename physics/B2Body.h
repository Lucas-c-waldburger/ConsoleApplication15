#pragma once
#include <unordered_set>
#include <cassert>
#include "B2Shape.h"
#include "../core/Result.h"

struct B2BodyDefinition
{
    B2BodyDefinition() : bodyData(b2DefaultBodyDef()) {}
    b2BodyDef bodyData;
    std::vector<B2ShapeDefinition> shapeDatas;
};

class B2Body
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

    B2Body() = default;
    explicit B2Body(const Handle<B2Body>& handle) : bodyHandle_(handle) {}
     
    void Destroy()
    {
        b2DestroyBody(bodyHandle_);
        shapeHandles_.clear();
        bodyHandle_ = Handle<B2Body>{};
    }

    bool IsValid() const { return b2Body_IsValid(bodyHandle_); }

    Type GetType() const { return static_cast<Type>(b2Body_GetType(bodyHandle_)); }

    void SetFixedRotation(bool fixed) { b2Body_SetFixedRotation(bodyHandle_, fixed); }
    bool IsFixedRotation() const { return b2Body_IsFixedRotation(bodyHandle_); }

    void SetAwake(bool awake) { b2Body_SetAwake(bodyHandle_, awake); }
    bool IsAwake() const { return b2Body_IsAwake(bodyHandle_); }

    SDL_FPoint GetPosition() const { return ToSDLFPoint(b2Body_GetPosition(bodyHandle_)); }
    void SetPosition(SDL_FPoint newPos, bool wakeState = true)
    {
        b2Body_SetTransform(bodyHandle_, ToB2Vec(newPos), b2Body_GetRotation(bodyHandle_));
        SetAwake(wakeState);
    }

    float GetAngle() const { return b2Rot_GetAngle(b2Body_GetRotation(bodyHandle_)); }
    void SetAngle(float newAngle, bool wakeState = true)
    {
        b2Body_SetTransform(bodyHandle_, b2Body_GetPosition(bodyHandle_), AngleToB2Rot(newAngle));
        SetAwake(wakeState);
    }

    SDL_FPoint GetVelocity() const { return ToSDLFPoint(b2Body_GetLinearVelocity(bodyHandle_)); }

    void ApplyForce(SDL_FPoint force, std::optional<SDL_FPoint> pointOfContact = {})
    {
        if (!pointOfContact.has_value())
        {
            ApplyForceToCenter(force);
        }
        else
        {
            b2Body_ApplyForce(bodyHandle_, ToB2Vec(force), ToB2Vec(*pointOfContact), true);
        }
    }

    void ApplyForceToCenter(SDL_FPoint force) { b2Body_ApplyForceToCenter(bodyHandle_, ToB2Vec(force), true); }

    void ApplyLinearImpulse(SDL_FPoint impulse, std::optional<SDL_FPoint> pointOfContact = {})
    {
        if (!pointOfContact.has_value())
        {
            ApplyLinearImpulseToCenter(impulse);
        }
        else
        {
            b2Body_ApplyLinearImpulse(bodyHandle_, ToB2Vec(impulse), ToB2Vec(*pointOfContact), true);
        }
    }

    void ApplyLinearImpulseToCenter(SDL_FPoint impulse)
    { 
        b2Body_ApplyLinearImpulseToCenter(bodyHandle_, ToB2Vec(impulse), true);
    }

    // Shapes API
    int GetShapeCount() const { return b2Body_GetShapeCount(bodyHandle_); }

    template <typename T> requires IsDerivedShape<T>
    Result<T> GetShape(const Handle<B2Shape>& shapeHandle)
    {
        if (!IsValid())
        {
            return MAKE_ERROR("BodyId was invalid");
        }
        if (!shapeHandle.IsValid())
        {
            return MAKE_ERROR("ShapeId was invalid");
        }

        auto it = shapeHandles_.find(shapeHandle);
        if (it == shapeHandles_.end())
        {
            return MAKE_ERROR("ShapeId not found on body");
        }

        if (!ShapeTypeMatches<T>(*it))
        {
            return MAKE_ERROR("T::shapeType differs from held shape's type");
        }
        
        return T{ *it };
    }

    Result<Handle<B2Shape>> AddShape(const B2ShapeDefinition& shapeDef);

    template <typename T> requires IsDerivedShape<T>
    Result<T> AddShape(const B2ShapeDefinition& shapeDef)
    {
        if (GetShapeCount() >= kMaxShapesPerBody)
        {
            return MAKE_ERROR_FMT("Body cannot have more than {} shapes attached", kMaxShapesPerBody);
        }

        b2ShapeId shapeId = b2_nullShapeId;

        if constexpr (std::same_as<T, B2PolygonShape>)
        {
            TRY(AddPolygon(bodyHandle_, shapeDef), id);
            shapeId = id;
        }

        Handle<B2Shape> shapeHandle = HandleFactory<B2Shape>::GetHandle(shapeId);

        assert(shapeHandle.IsValid());
        assert(shapeHandles_.insert(shapeHandle).second);

        return T{ shapeHandle };
    }

private:
    static Result<b2ShapeId> AddPolygon(b2BodyId bodyId, const B2ShapeDefinition& shapeDef)
    {
        if (shapeDef.data.dimensions.has_value())
        {
            return AddBox(bodyId, shapeDef);
        }
        
        return AddPolygonImpl(bodyId, shapeDef);
    }

    static Result<b2ShapeId> AddBox(b2BodyId bodyId, const B2ShapeDefinition& shapeDef)
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

    static Result<b2ShapeId> AddPolygonImpl(b2BodyId bodyId, const B2ShapeDefinition& shapeDef)
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

    Handle<B2Body> bodyHandle_;
    std::unordered_set<Handle<B2Shape>> shapeHandles_;
};