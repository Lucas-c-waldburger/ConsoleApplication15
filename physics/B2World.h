#pragma once
#include <SDL.h>
#include "B2RayCast.h"

// TODO: have timestep/substepCount be member vars
class B2World
{
public:
    B2World() = default;

    static B2World Create(float gravX, float gravY); 

    bool IsValid() const { return b2World_IsValid(worldId_); }

    void Destroy();

    Result<Void> Step(float timeStep, int subStepCount);

    SDL_FPoint GetGravity() const;

    void SetGravity(float newX, float newY);

    b2WorldId GetID() const { return worldId_; }

    Result<B2Body> AddBody(const B2BodyDefinition& bodyDef);

    Result<B2Body> GetBody(const Handle<B2Body>& bodyHandle) const;

    const B2RayCastContext& CastRay(const B2Body& bodyA, const B2Body& bodyB);

    B2RayCastResult CastRayClosest(const B2Body& bodyA, const B2Body& bodyB);

    B2RayCastResult CastRayToPoint(const B2Body& body, SDL_FPoint point);

private:
    b2WorldId worldId_ = b2_nullWorldId;
    B2RayCaster rayCaster_;
};