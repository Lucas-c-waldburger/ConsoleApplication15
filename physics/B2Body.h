#pragma once
#include <unordered_set>
#include <cassert>
#include "B2Shape.h"
#include "../core/Result.h"
#include "../core/HandleFactory.h"


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
        bodyHandle_ = Handle<B2Body>{};
    }

    bool IsValid() const { return b2Body_IsValid(bodyHandle_); }

    const Handle<B2Body>& GetHandle() const { return bodyHandle_; }

    Type GetBodyType() const { return static_cast<Type>(b2Body_GetType(bodyHandle_)); }

    void SetFixedRotation(bool fixed) { b2Body_SetFixedRotation(bodyHandle_, fixed); }
    bool IsFixedRotation() const { return b2Body_IsFixedRotation(bodyHandle_); }

    void SetAwake(bool awake) { b2Body_SetAwake(bodyHandle_, awake); }
    bool IsAwake() const { return b2Body_IsAwake(bodyHandle_); }

    SDL_FPoint GetPosition() const { return ToSDLFPointScaled(b2Body_GetPosition(bodyHandle_)); }
    void SetPosition(SDL_FPoint newPos, bool wakeState = true)
    {
        b2Body_SetTransform(bodyHandle_, ToB2VecScaled(newPos), b2Body_GetRotation(bodyHandle_));
        SetAwake(wakeState);
    }

    float GetAngle() const { return b2Rot_GetAngle(b2Body_GetRotation(bodyHandle_)); }
    void SetAngle(float newAngle, bool wakeState = true)
    {
        b2Body_SetTransform(bodyHandle_, b2Body_GetPosition(bodyHandle_), AngleToB2Rot(newAngle));
        SetAwake(wakeState);
    }

    SDL_FPoint GetLinearVelocity() const { return ToSDLFPoint(b2Body_GetLinearVelocity(bodyHandle_)); }
    void SetLinearVelocity(SDL_FPoint newVel) { b2Body_SetLinearVelocity(bodyHandle_, ToB2Vec(newVel)); }

    float GetAngularVelocity() const { return b2Body_GetAngularVelocity(bodyHandle_); }
    void SetAngularVelocity(float newVel) { b2Body_SetAngularVelocity(bodyHandle_, newVel); }

    void ApplyForce(SDL_FPoint forceNewtons, std::optional<SDL_FPoint> worldPoint = {})
    {
        if (!worldPoint.has_value())
        {
            ApplyForceToCenter(forceNewtons);
        }
        else
        {
            b2Body_ApplyForce(bodyHandle_, ToB2Vec(forceNewtons), ToB2VecScaled(*worldPoint), true);
        }
    }

    void ApplyForceToCenter(SDL_FPoint forceNewtons) 
    { 
        b2Body_ApplyForceToCenter(bodyHandle_, ToB2Vec(forceNewtons), true); 
    }

    void ApplyLinearImpulse(SDL_FPoint impulse, std::optional<SDL_FPoint> worldPoint = {})
    {
        if (!worldPoint.has_value())
        {
            ApplyLinearImpulseToCenter(impulse);
        }
        else
        {
            b2Body_ApplyLinearImpulse(bodyHandle_, ToB2Vec(impulse), ToB2VecScaled(*worldPoint), true);
        }
    }

    void ApplyLinearImpulseToCenter(SDL_FPoint impulse)
    { 
        b2Body_ApplyLinearImpulseToCenter(bodyHandle_, ToB2Vec(impulse), true);
    }

    float GetDistance(const B2Body& other) const
    {
        if (!(IsValid() && other.IsValid()))
        {
            return std::numeric_limits<float>::min();
        }

        return b2Distance(b2Body_GetPosition(bodyHandle_), b2Body_GetPosition(other.bodyHandle_));
    }

    float GetDistance(SDL_FPoint point) const
    {
        if (!IsValid())
        {
            return std::numeric_limits<float>::min();
        }

        return b2Distance(b2Body_GetPosition(bodyHandle_), ToB2VecScaled(point));
    }

    // Shapes API
    int GetShapeCount() const { return b2Body_GetShapeCount(bodyHandle_); }

    Result<B2Shape> GetShape(const Handle<B2Shape>& shapeHandle);

    template <typename T> requires SomeDerivedB2Shape<T>
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
        if (!OwnsShape(shapeHandle))
        {
            return MAKE_ERROR("ShapeId not found on body");
        }
        if (!ShapeTypeMatches<T>(shapeHandle))
        {
            return MAKE_ERROR("T::shapeType differs from held shape's type");
        }
        
        return T{ shapeHandle };
    }

    Result<B2Shape> AddShape(const B2ShapeDefinition& shapeDef);

    std::unordered_set<Handle<B2Shape>> GetShapeHandles() const;

    bool OwnsShape(const Handle<B2Shape>& shapeHandle) const;

private:
    static Result<b2ShapeId> AddCircle(b2BodyId bodyId, const B2ShapeDefinition& shapeDef);
    static Result<b2ShapeId> AddPolygon(b2BodyId bodyId, const B2ShapeDefinition& shapeDef);   
    static Result<b2ShapeId> AddPolygonImpl(b2BodyId bodyId, const B2ShapeDefinition& shapeDef);
    static Result<b2ShapeId> AddBox(b2BodyId bodyId, const B2ShapeDefinition& shapeDef);

    Handle<B2Body> bodyHandle_;
};

