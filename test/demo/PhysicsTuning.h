#pragma once
#include "../../ecs/Ecs.h"
#include "platformer/Common.h"

namespace test {

struct PhysicsParameters
{
    // --- Horizontal movement ---
    float maxSpeed = 6.0f;
    float acceleration = 40.0f;
    float deceleration = 60.0f;
    float airAcceleration = 20.0f;

    // --- Jump ---
    float jumpVelocity = 10.0f;

    // --- Gravity shaping ---
    float baseGravityScale = 1.0f;
    float fallGravityScale = 1.8f;
    float lowJumpGravityScale = 2.5f;
    float apexGravityScale = 0.5f;
    float apexThreshold = 1.0f;

    // --- Jump feel ---
    float jumpCutoffFactor = 0.5f;

    // --- Forgiveness ---
    float coyoteTime = 0.1f;
    float jumpBufferTime = 0.1f;

    // --- Damping ---
    float groundDamping = 8.0f;
    float airDamping = 1.5f;
};

enum JumpFlag : uint8_t
{
    Pressed = 1 << 0,
    Held = 1 << 1,
    Released = 1 << 2
};

struct InputFrame
{
    float moveAxis = 0.0f;
    uint8_t jumpFlag = 0;
};

struct JumpTimers
{
    float coyote = 0.0f;
    float jumpBuffer = 0.0f;
};

struct PhysicsState
{
    bool grounded = false;
    uint8_t jumpFlag = 0;
    JumpTimers jumpTimers;
};

static void UpdateJumpTimers(PhysicsState& s, const PhysicsParameters& t, 
                             const InputFrame& input, float dt)
{
    if (s.grounded)
    {
        s.jumpTimers.coyote = t.coyoteTime;
    }
    else
    {
        s.jumpTimers.coyote = std::max(0.0f, s.jumpTimers.coyote - dt);
    }

    if (input.jumpFlag & JumpFlag::Pressed)
    {
        s.jumpTimers.jumpBuffer = t.jumpBufferTime;
    }
    else
    {
        s.jumpTimers.jumpBuffer = std::max(0.0f, s.jumpTimers.jumpBuffer - dt);
    }
}

static void ApplyHorizontalMovement(RigidBody& rigid, PhysicsState& s, 
                                    const PhysicsParameters& t,
                                    const InputFrame& input)
{
    auto& body = rigid.body.GetData();

    float targetSpeed = input.moveAxis * t.maxSpeed;
    float currentSpeed = body.GetLinearVelocity().x;

    float delta = targetSpeed - currentSpeed;

    float accel = (std::abs(input.moveAxis) > 0.01f)
        ? (s.grounded ? t.acceleration : t.airAcceleration)
        : t.deceleration;

    float forceX = delta * accel * body.GetMass();

    rigid.forceRequests.forces.emplace_back(Force{ .value = { forceX, 0.0f } });
}

static void ApplyJump(RigidBody& rigid, PhysicsState& s, const PhysicsParameters& t)
{
    bool canJump = s.jumpTimers.coyote > 0.0f;
    bool buffered = s.jumpTimers.jumpBuffer > 0.0f;

    if (canJump && buffered)
    {
        auto& body = rigid.body.GetData();

        auto vel = body.GetLinearVelocity();

        // Reset downward velocity for consistent jump
        if (vel.y > 0.0f)
        {
            vel.y = 0.0f;
            WriteAccessor<B2Body>{}(rigid.body).SetLinearVelocity(vel);
        }

        float impulse = body.GetMass() * t.jumpVelocity;

        rigid.forceRequests.impulses.emplace_back(Force{ .value = { 0.0f, -impulse } });

        s.jumpTimers.coyote = 0.0f;
        s.jumpTimers.jumpBuffer = 0.0f;
    }
}

static void ApplyJumpCutoff(RigidBody& rigid, const PhysicsParameters& t,
                            const InputFrame& input)
{
    if ((input.jumpFlag & JumpFlag::Released) == 0)
        return;

    auto vel = rigid.body.GetData().GetLinearVelocity();

    if (vel.y < 0.0f)
    {
        vel.y *= t.jumpCutoffFactor;

        WriteAccessor<B2Body>{}(rigid.body).SetLinearVelocity(vel);
    }
}

static void ApplyGravityScale(RigidBody& rigid, const PhysicsParameters& t,
                              const InputFrame& input)
{
    auto vel = rigid.body.GetData().GetLinearVelocity();

    float scale = t.baseGravityScale;

    if (vel.y > 0.0f)
    {
        scale *= t.fallGravityScale;
    }
    else if ((input.jumpFlag & JumpFlag::Held) == 0)
    {
        scale *= t.lowJumpGravityScale;
    }

    if (std::abs(vel.y) < t.apexThreshold)
    {
        scale *= t.apexGravityScale;
    }

    WriteAccessor<B2Body>{}(rigid.body).SetGravityScale(scale);
}

static void ApplyDamping(RigidBody& rigid, PhysicsState& s, const PhysicsParameters& t)
{
    WriteAccessor<B2Body>{}(rigid.body).SetLinearDamping(
        (s.grounded ? t.groundDamping : t.airDamping)
    );
}

static void UpdatePhysics(
    RigidBody& rigid,
    PhysicsState& s,
    const PhysicsParameters& t,
    const InputFrame& input,
    float dt)
{
    UpdateJumpTimers(s, t, input, dt);
    ApplyHorizontalMovement(rigid, s, t, input);
    ApplyJump(rigid, s, t);
    ApplyJumpCutoff(rigid, t, input);
    ApplyGravityScale(rigid, t, input);
    ApplyDamping(rigid, s, t);
}

static void SetUpPhysicsTuningEntity(Entity& e, EventBus& bus)
{

}

class PhysicsTuningSystem
{
public:
    PhysicsTuningSystem(const Entity& e, const PhysicsParameters& params) : 
        entityId_(e.GetID()), physicsParams_(params) {}

    void Update(float dt)
    {
        auto e = ECS::GetEntityByID(entityId_);
        if (!e.IsValid())
        {
            return;
        }

        auto [rigid, physState, input] = 
            e.GetComponents<RigidBody, PhysicsState, InputFrame>();

        UpdatePhysics(rigid, physState, physicsParams_, input, dt);
    }

    PhysicsParameters& GetPhysicsParameters() { return physicsParams_; }
    const PhysicsParameters& GetPhysicsParameters() const { return physicsParams_; }

private:
    Entity_t entityId_ = kInvalidEntity;
    PhysicsParameters physicsParams_;
};

} // test