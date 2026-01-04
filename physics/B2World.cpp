#include "B2World.h"
#include "../core/HandleFactory.h"
#include <cassert>

B2World B2World::Create(float gravX, float gravY)
{
    B2World world;

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { gravX, gravY };

    world.worldId_ = b2CreateWorld(&worldDef);
    assert(b2World_IsValid(world.worldId_));

    return world;
}

void B2World::Destroy()
{
    if (IsValid())
    {
        b2DestroyWorld(worldId_);
    }
}

Result<Void> B2World::Step(float timeStep, int subStepCount)
{
    if (!IsValid())
    {
        return MAKE_ERROR("WorldId was invalid");
    }

    b2World_Step(worldId_, timeStep, subStepCount);

    return Void{};
}

SDL_FPoint B2World::GetGravity() const
{
    b2Vec2 grav = b2World_GetGravity(worldId_);

    return { grav.x, grav.y };
}

void B2World::SetGravity(float newX, float newY)
{
    b2Vec2 newGrav = { newX, newY };

    return b2World_SetGravity(worldId_, newGrav);
}

const B2RayCastContext& B2World::CastRay(const B2Body& bodyA, const B2Body& bodyB)
{
    if (!(IsValid() && bodyA.IsValid() && bodyB.IsValid()))
    {
        rayCaster_.ClearContext();
        return rayCaster_.GetContext();
    }

    return rayCaster_.CastRay(worldId_, bodyA.GetPosition(), bodyB.GetPosition(), 
                              B2RayCastCallback::FindAllShapes);
}

B2RayCastResult B2World::CastRayClosest(const B2Body& bodyA, const B2Body& bodyB)
{
    if (!(IsValid() && bodyA.IsValid() && bodyB.IsValid()))
    {
        return {};
    }

    return rayCaster_.CastRayClosest(worldId_, bodyA.GetPosition(), bodyB.GetPosition());
}

B2RayCastResult B2World::CastRayToPoint(const B2Body& body, SDL_FPoint point)
{
    if (!(IsValid() && body.IsValid()))
    {
        return {};
    }

    return rayCaster_.CastRayClosest(worldId_, body.GetPosition(), point);
}

void B2World::Explode(const B2ExplosionDefinition& expDef)
{
    if (!IsValid())
    {
        return;
    }

    b2ExplosionDef b2Def = b2DefaultExplosionDef();
    b2Def.maskBits = expDef.categoryBitMask;
    b2Def.position = ToB2VecScaled(expDef.position);
    b2Def.radius = expDef.radius;
    b2Def.falloff = expDef.falloff;
    b2Def.impulsePerLength = expDef.impulsePerLength;

    b2World_Explode(worldId_, &b2Def);
}

Result<B2Body> B2World::AddBody(const B2BodyDefinition& bodyDef)
{
    if (!IsValid())
    {
        return MAKE_ERROR("WorldId was invalid");
    }

    b2BodyId bodyId = b2CreateBody(worldId_, &bodyDef.bodyData);
    Handle<B2Body> bodyHandle = Handle<B2Body>::Create(bodyId);

    assert(bodyHandle.IsValid());

    B2Body body{ bodyHandle };

    for (const auto& shapeDef : bodyDef.shapeDatas)
    {
        TRY(body.AddShape(shapeDef));
    }

    return body;
}

Result<B2Body> B2World::GetBody(const Handle<B2Body>& bodyHandle) const
{
    if (!IsValid())
    {
        return MAKE_ERROR("WorldId was invalid");
    }
    if (!bodyHandle.IsValid())
    {
        return MAKE_ERROR("BodyId was invalid");
    }
    if (b2Body_GetWorld(bodyHandle) != worldId_)
    {
        return MAKE_ERROR("BodyId does not belong to world");
    }

    return B2Body{ bodyHandle };
}