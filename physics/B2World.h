#pragma once
#include <SDL.h>
#include "B2Body.h"

inline constexpr bool operator==(const b2WorldId& lhs, const b2WorldId& rhs)
{
    return lhs.generation == rhs.generation && lhs.index1 == rhs.index1;
}

class B2World
{
public:
    static B2World Create(float gravX, float gravY);

    bool IsValid() const { return b2World_IsValid(worldId_); }

    void Destroy();

    Result<Void> Step(float timeStep, int subStepCount);

    SDL_FPoint GetGravity() const;

    void SetGravity(float newX, float newY);

    b2WorldId GetID() const { return worldId_; }

    Result<B2Body> AddBody(const B2BodyDefinition& bodyDef);

    Result<B2Body> GetBody(const Handle<B2Body>& bodyHandle) const;

private:
    B2World() = default;
    b2WorldId worldId_ = b2_nullWorldId;
};