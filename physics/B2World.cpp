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

Result<B2Body> B2World::AddBody(const B2BodyDefinition& bodyDef)
{
    if (!IsValid())
    {
        return MAKE_ERROR("WorldId was invalid");
    }

    b2BodyId bodyId = b2CreateBody(worldId_, &bodyDef.bodyData);
    Handle<B2Body> bodyHandle = HandleFactory<B2Body>::GetHandle(bodyId);

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