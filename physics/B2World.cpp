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
    assert(!bodyHandles_.contains(bodyHandle));

    B2Body body{ bodyHandle };

    for (const auto& shapeDef : bodyDef.shapeDatas)
    {
        TRY(body.AddShape(shapeDef));
    }

    assert(bodyHandles_.insert(bodyHandle).second);

    return body;
}

Result<B2Body> B2World::GetBody(const Handle<B2Body>& bodyHandle)
{
    if (!IsValid())
    {
        return MAKE_ERROR("WorldId was invalid");
    }
    if (!bodyHandle.IsValid())
    {
        return MAKE_ERROR("BodyId was invalid");
    }
    if (!bodyHandles_.contains(bodyHandle))
    {
        return MAKE_ERROR("BodyId does not belong to world");
    }

    B2Body body{ bodyHandle };

    b2ShapeId shapeIds[B2Body::kMaxShapesPerBody];
    int count = b2Body_GetShapes(bodyHandle, shapeIds, B2Body::kMaxShapesPerBody);
    body.shapeHandles_.reserve(count);

    for (int i = 0; i < count; i++)
    {
        Handle<B2Shape> shapeHandle = HandleFactory<B2Shape>::GetHandle(shapeIds[i]);

        if (shapeHandle.IsValid())
        {
            body.shapeHandles_.insert(shapeHandle);
        }
        else
        {
            LOG_WARNING("Shape handle was invalid");
        }
    }

    return body;
}