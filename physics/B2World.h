#pragma once
#include <SDL.h>
#include "B2Body.h"

class B2World
{
public:
    static B2World Create(float gravX, float gravY);

    bool IsValid() const { return b2World_IsValid(worldId_); }

    void Destroy();

    SDL_FPoint GetGravity() const;

    void SetGravity(float newX, float newY);

    b2WorldId GetID() const { return worldId_; }

    Result<B2Body> AddBody(const B2BodyDefinition& bodyDef);

    Result<B2Body> GetBody(const Handle<B2Body>& bodyHandle);

private:
    B2World() = default;
    b2WorldId worldId_ = b2_nullWorldId;
    std::unordered_set<Handle<B2Body>> bodyHandles_;
};