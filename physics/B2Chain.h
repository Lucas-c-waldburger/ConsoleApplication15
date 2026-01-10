#pragma once

#include "B2Handle.h"
#include "B2Utils.h"
#include "B2CollisionFilter.h"

struct B2ChainMaterial
{
    float friction = 0.5f;
    float restitution = 0.0f;
    float rollingResistance = 0.0f;
    float tangentSpeed = 0.0f;

    operator b2SurfaceMaterial() const {
        auto mat = b2DefaultSurfaceMaterial();
        mat.friction = friction;
        mat.restitution = restitution;
        mat.rollingResistance = rollingResistance;
        mat.tangentSpeed = tangentSpeed;

        return mat;
    }
};

struct B2ChainDefinition
{
    bool enableSensorEvents = false;
    std::vector<SDL_FPoint> points;
    B2CollisionFilter collisionFilter{};
    bool isLoop = false;
    std::vector<B2ChainMaterial> materials{};
};

class B2ChainSegmentShape;

class B2Chain
{
public:
    B2Chain() = default;
    explicit B2Chain(const Handle<B2Chain>& handle) : chainHandle_(handle) {}

    bool operator==(const B2Chain& rhs) const { return chainHandle_ == rhs.chainHandle_; }

    const Handle<B2Chain>& GetHandle() const { return chainHandle_; }

    bool IsValid() const { return chainHandle_.IsValid(); }

    void Destroy() 
    { 
        b2DestroyChain(chainHandle_); 
        chainHandle_ = {};
    }

    float GetFriction() const
    {
        return b2Chain_GetFriction(chainHandle_);
    }
    void SetFriction(float friction)
    {
        b2Chain_SetFriction(chainHandle_, friction);
    }

    float GetRestitution() const
    {
        return b2Chain_GetRestitution(chainHandle_);
    }
    void SetRestitution(float restitution)
    {
        b2Chain_SetFriction(chainHandle_, restitution);
    }

    size_t GetSegmentCount() const
    {
        return static_cast<size_t>(b2Chain_GetSegmentCount(chainHandle_));
    }

    std::vector<B2ChainSegmentShape> GetSegments() const;

private:
	Handle<B2Chain> chainHandle_;
};
