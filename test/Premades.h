#pragma once
#include <format>
#include "../ecs/Ecs.h"
#include "../core/Conversions.h"
#include "../scripting/ScriptManager.h"
#include "../sdl/SDLUtils.h"
#include "../sdl/SDLite.h"
#include "../systems/PhysicsSystem.h"
#include "../systems/CameraSystem.h"
#include "../systems/RenderSystem.h"
#include "../systems/PhysicsSystem.h"
#include "../systems/EventCallbackSystem.h"
#include "../components/builder/RigidBodyComponentBuilder.h"
#include "../components/builder/ColliderComponentBuilder.h"
#include "../events/EventBus2.h"
//#include "../callbacks/StateTransitionCallbackRegistry.h"
//#include "callbacks/AnimationCallbacks.h"
//#include "callbacks/GameControllerCallbacks.h"

static Result<Entity> MakeColliderBoxEntity(B2World& world, SDL_FPoint position, Dimensions<float> dimensions, 
                                            B2Body::Type bodyType, const ColliderSettings& settings = {},
                                            SDL_Color color = SDLite::kColorBlack)
{
    if (!world.IsValid())
    {
        return MAKE_ERROR("B2World was invalid");
    }

	Entity entity = ECS::CreateEntity();
	assert(entity.IsValid());

    entity.AddComponent(Transform{});

    auto& rigidBody = entity.AddComponent(ComponentBuilder<RigidBody>{}.WithBodyParameters({
        .bodyType = bodyType,
        .position = position
    }).Build(world));

    auto bodyOc = world.GetBody(rigidBody.body.GetData().GetHandle());
    if (!bodyOc.Success())
    {
        entity.Destroy();
        return bodyOc.GetError();
    }
    auto& body = bodyOc.GetValue();
    assert(body.IsValid());
    
    entity.AddComponent(ComponentBuilder<Collider>{}
        .WithShapeParameters({
            .shapeType = B2Shape::Type::Polygon,
            .dimensions = dimensions
        })
        .WithColliderSettings(settings)
        .Build(body));

    RenderProfile profile{
        .debugDraw = {
            .boundingBox = {.on = true },
            .collider = {.on = true }
        }
    };
    entity.AddComponent(Renderable{ .profile = std::move(profile) });

	return entity;
}

static Result<Entity> MakeColliderCircleEntity(B2World& world, SDL_FPoint position, float radius,
                                               B2Body::Type bodyType, const ColliderSettings& settings = {},
                                               SDL_Color color = SDLite::kColorBlack)
{
    if (!world.IsValid())
    {
        return MAKE_ERROR("B2World was invalid");
    }

    Entity entity = ECS::CreateEntity();
    assert(entity.IsValid());

    entity.AddComponent(Transform{});

    auto& rigidBody = entity.AddComponent(ComponentBuilder<RigidBody>{}
    .WithBodyParameters({
        .bodyType = bodyType,
        .position = position
    }).Build(world));

    auto bodyOc = world.GetBody(rigidBody.body.GetData().GetHandle());
    if (!bodyOc.Success())
    {
        entity.Destroy();
        return bodyOc.GetError();
    }
    auto& body = bodyOc.GetValue();
    assert(body.IsValid());

    entity.AddComponent(ComponentBuilder<Collider>{}
    .WithShapeParameters({
        .shapeType = B2Shape::Type::Circle,
        .radius = radius
    })
    .WithColliderSettings(settings)
    .Build(body));

    RenderProfile profile{
        .debugDraw = {
            .boundingBox = {.on = true },
            .collider = {.on = true }
        }
    };
    entity.AddComponent(Renderable{ .profile = std::move(profile) });

    return entity;
}

struct SpoofBodyAccessor : public HasWriteAccess<SpoofBodyAccessor, B2Body>
{
    B2Body& operator()(ReadOnly<B2Body>& ro)
    {
        return GetWriteAccess(ro);
    }
};

inline Result<Entity> MakeMultiColliderEntity(B2World& world)
{
    TRY(MakeColliderBoxEntity(world, { 200.0f, 200.0f }, { 50.0f, 50.0f }, 
        B2Body::Type::Dynamic, 
        ColliderSettings{ .restitution = 0.9f, .enableEvents{ .contact = true } }, SDLite::kColorGreen),
    parentEnt);
    assert(parentEnt.IsValid());

    auto& parentRigidBody = parentEnt.GetComponent<RigidBody>();
    assert(parentRigidBody.body.GetData().IsValid());

    auto& parentBodyMutable = SpoofBodyAccessor{}(parentRigidBody.body);

    auto parentRelations = parentEnt.GetRelations();
    auto childEnt = parentRelations.AddChild();
    assert(childEnt.IsValid());
    assert(parentRelations.IsParent());
    assert(parentRelations.IsParentOf(childEnt));

    auto childRelations = childEnt.GetRelations();
    assert(childRelations.IsChild());
    assert(childRelations.IsChildOf(parentEnt));

    childEnt.AddComponent(Transform{});

    childEnt.AddComponent(ComponentBuilder<Collider>{}
    .WithShapeParameters({
        .shapeType = B2Shape::Type::Circle,
        .localPosition = SDL_FPoint{ -50.0f, 0.0f },
        .radius = 20.0f
    })
    .WithColliderSettings({
        .restitution = 0.9f,
        .enableEvents{ .contact = true }
    }).Build(parentBodyMutable));

    RenderProfile profile{
        .debugDraw = {
            .boundingBox = {.on = true },
            .collider = {.on = true }
        }
    };
    childEnt.AddComponent(Renderable{ .profile = std::move(profile) });

    auto allChildren = parentRelations.GetChildren();
    assert(allChildren.size() == 1);
    assert(allChildren[0].GetID() == childEnt.GetID());
    

    return parentEnt;
}


namespace test {
    
inline SDL_FPoint UpdateMouseTextEnt(Entity& entity, SDL_FPoint lastMousePos)
{
    assert(entity.IsValid());
    assert((entity.HasComponents<MouseState, Renderable, Transform>()));

    auto [mouse, renderable, transform] = 
        entity.GetComponents<MouseState, Renderable, Transform>();

    auto mousePos = mouse.cursorValue.position.absolute;

    if (mousePos == lastMousePos || transform.position == mousePos)
    {
        return mousePos;
    }

    transform.position = mouse.cursorValue.position.absolute;

    assert(std::holds_alternative<TextRenderable>(renderable.renderData));

    auto& textData = std::get<TextRenderable>(renderable.renderData);

    textData.text = std::format("x: {}\ny: {}", transform.position.x, transform.position.y);

    textData.flags |= TextRenderable::Flag::DirtyText;

    return mousePos;
}


//static constexpr float kMaxImpulseValue = 8.0f;
//static constexpr float kImpuseScale = kMaxImpulseValue / 32768.0f;
//
//class PremadeCallbacks
//{
//public:
//    using MapType = std::unordered_map<std::string, 
//        Handle<EventCallbackKey>, TransparentStringHash, std::equal_to<>>;
//
//    explicit PremadeCallbacks(EventCallbackRegistry& registry) : registry_(registry) {}
//    
//    MapType Create();
//
//    template <typename Fn>
//    std::pair<std::string, Handle<EventCallbackKey>>
//    Register(const char* callbackNm, Fn&& fn)
//    {
//        return std::make_pair(
//            callbackNm,
//            registry_.RegisterCallback(callbackNm, std::forward<Fn>(fn))
//        );
//    }
//
//private:
//    EventCallbackRegistry& registry_;
//};
//
//PremadeCallbacks::MapType PremadeCallbacks::Create() {
//
//    return PremadeCallbacks::MapType
//    {
//    Register(NAME_AND_CALL(ConnectToFirstController)),
//    Register(NAME_AND_CALL(DisconnectController)),
//    Register(NAME_AND_CALL(ApplyAxisInputToForce, kImpuseScale)),
//    Register(NAME_AND_CALL(SpriteAdvanceOnDistanceTraveled, 30))
//    };
//};

//enum class RequestAction
//{
//    Accumulate,
//    Set
//};
//
//template <typename T>
//struct RequestActionQueue
//{
//    const T currentValue;
//    std::vector<std::pair<RequestAction, T>> requestQueue;
//};

struct PhysicsData
{
    SDL_FPoint position = { 0.0f, 0.0f };
    float rotation = 0.0f;
    struct { float linear = 0.0f, angular = 0.0f; } velocity;
    float mass = 0.0f;
    float gravityScale = 0.0f;
};

//struct ParticleParameters
//{
//    float lifetime = 0.0f;
//    Extent<SDL_FPoint> scale = {{ 1.0f }, { 1.0f }};
//
//};
//
//struct ParticleBehaviors
//{
//    template <typename T>
//    using Behavior = fu2::function_view<void(Entity&)>
//};

struct ParticleProfile
{
    float lifetime             = 0.0f;
    SDL_FPoint drift           = { 0.0f, 0.0f };
    SDL_FPoint emitterOffset   = { 0.0f, 0.0f };
    Extent<SDL_Color> colorMod = {{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }};
    Extent<SDL_FPoint> scale   = {{ 1.0f, 1.0f }, { 1.0f, 1.0f }};
    Extent<float> rotation     = { 0.0f, 0.0f };
};

using ParticleGenerator = fu2::unique_function<ParticleProfile()>;




class ParticleEmitter
{
private:
    auto GetSpawnerLambda()
    {
        return [this](const events::TimerFired& ev) {
            if (ev.producer != emitter_.GetID())
            {
                return;
            }

            if (particles_.size() < maxParticles_)
            {
                AddParticle();
            }
        };
    }

    static ParticleGenerator GetDefaultParticleGenerator()
    {
        return []() {
            return ParticleProfile{
                .lifetime = 0.5f + (rand() % 100) / 100.0f,
                .drift = {.x = (rand() % 21 - 10) * 0.5f,
                          .y = (rand() % 21 - 10) * 0.5f },
                .scale = {.start = 0.5f, .end = 2.0f }
            };
        };
    };

public:
    ParticleEmitter(EventBus2& bus, Renderable renderable, SDL_FPoint emitterPos,
        ParticleGenerator&& generator = GetDefaultParticleGenerator(), 
        int particlesPerSec = 15, 
        size_t maxCount = 10);
    ~ParticleEmitter();

    void AddParticle()
    {
        if (liveCount_ >= maxParticles_)
        {
            return;
        }

        if (!particleGenerator_)
        {
            LOG_ERROR("No particle generator set!");
            return;
        }

        auto& [particle, bx] = particles_[liveCount_++];
        if (!particle.IsValid())
        {
            particle = ECS::CreateEntity();

            // stop them from firing when destroyed
            particle.SetEventProduction<events::TimerFired>(false);
            particle.SetEventProduction<events::EntityDestroyed>(false);
        }

        particle.SetComponentVisibility(true);    

        bx = particleGenerator_();

        const SDL_FPoint emitterPos = emitter_.GetComponent<Transform>().position;

        auto& tf = particle.AddComponent<Transform>();
        tf.position = emitterPos + bx.emitterOffset;
        tf.scale = bx.scale.start;      

        auto& timer = particle.AddComponent<Timer>();
        timer.duration = bx.lifetime;
        timer.flags = Timer::Flag::Active;

        particle.AddComponent<Renderable>() = renderable_;
    }

    void Update()
    {
        for (size_t i = 0; i < liveCount_; i++)
        {
            auto& [entity, bx] = particles_[i];

            auto kill = [this, &entity, &i]() {
                // wont be processed by any system, will be considered IsValid()
                entity.SetComponentVisibility(false);

                if (--liveCount_ <= i)
                {
                    return;
                }

                std::swap(particles_[i], particles_[liveCount_]);
                --i;
            };

            if (!entity.IsValid())
            {
                kill();              
                continue;
            }

            auto [tf, timer, renderable] = 
                entity.GetComponents<Transform, Timer, Renderable>();

            if ((timer.flags & Timer::Flag::Active) == 0)
            {
                kill();
                continue;
            }

            assert(timer.duration > 0.0f);
            
            float t = timer.elapsed / timer.duration;
            uint8_t alpha = static_cast<uint8_t>(255 * (1.0f - t));

            renderable.profile.mods.alpha = alpha;

            float scale = 2.0f * (1.0f - t);

            tf.scale = { scale, scale };
            tf.position += bx.drift;
        }
    }

    Entity& GetEmitterEntity() { return emitter_; }

    size_t GetMaxParticles() const { return maxParticles_; }
    void SetMaxParticles(size_t newMax) { maxParticles_ = newMax; }

    float GetSpawnRate() const { return spawnRate_; }
    void SetSpawnRate(float newRate) { spawnRate_ = newRate; }

    template <typename Fn> requires std::convertible_to<Fn, ParticleGenerator>
    void SetParticleGenerator(Fn&& fn) { }

private:
    struct MovementBehavior 
    {
        SDL_FPoint drift = { 0.0f, 0.0f };
    };

    Renderable renderable_;
    size_t maxParticles_ = 0;
    float spawnRate_ = 0.0f;

    Entity emitter_;
    std::vector<std::pair<Entity, ParticleProfile>> particles_;
    ParticleGenerator particleGenerator_ = nullptr;
    size_t liveCount_ = 0;
};



} // test