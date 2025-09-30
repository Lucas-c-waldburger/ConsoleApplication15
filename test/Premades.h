#pragma once
#include <format>
#include "../ecs/Ecs.h"
#include "../core/Conversions.h"
#include "../components/RenderableComponent.h"
#include "../components/ParticleBehaviorComponent.h"
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
#include "../core/EvaluationProperty.h"
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

//template <typename T>
//struct ParticleBehaviorValue
//{
//    Extent<T> value;
//};



//template <typename T>
//inline constexpr EvaluationProperty<T> MakeFixedRateProperty(const T& rate)
//{
//    return EvaluationProperty{ EvaluationSpec::FixedRate, rate, {}};
//}
//template <typename T>
//inline constexpr EvaluationProperty<T> MakeInterpolatedProperty(const T& start, const T& end)
//{
//    return EvaluationProperty{ EvaluationSpec::Interpolated, start, end };
//}

//struct ParticleProfile
//{
//    float lifetime           = 0.0f;
//    SDL_FPoint drift         = { 0.0f, 0.0f };
//    SDL_FPoint emitterOffset = { 0.0f, 0.0f };
//    Extent<TextureMods> mods = {};
//    Extent<SDL_FPoint> scale = {{ 1.0f, 1.0f }, { 1.0f, 1.0f }};
//    Extent<float> rotation   = { 0.0f, 0.0f };
//};



using ParticleBehaviorGenerator = fu2::unique_function<ParticleBehavior()>;

class ParticleEmitter
{
public:
    template <typename R> requires std::constructible_from<Renderable::RenderData, R>
    ParticleEmitter(R renderData, SDL_FPoint emitterPos,
        ParticleBehaviorGenerator&& bxGenerator, uint32_t particlesPerSec = 15,
        size_t maxCount = 10) : emitter_(ECS::CreateEntity()), 
        bxGenerator_(std::move(bxGenerator)), maxParticles_(maxCount), particles_(maxCount)
    {
        assert(emitter_.IsValid());

        // just use to hold render data that particles will use
        emitter_.SetComponentVisibility<Renderable>(false);
        auto& emitterRenderData = emitter_.AddComponent<Renderable>().renderData;
        emitterRenderData = std::move(renderData);
        assert(!std::holds_alternative<std::monostate>(emitterRenderData));

        emitter_.AddComponent<Transform>().position = emitterPos;

        auto& spawnTimer = emitter_.AddComponent<Timer>();
        spawnTimer.duration = (particlesPerSec > 0) ?
            1.0f / static_cast<float>(particlesPerSec) : 1.0f;
        spawnTimer.flags = 0;
    }
    ~ParticleEmitter();

    void AddParticle();
    void Update(float delta);

    Entity& GetEmitterEntity() { return emitter_; }

    size_t GetMaxParticles() const { return maxParticles_; }
    void SetMaxParticles(size_t newMax) 
    { 
        if (newMax < maxParticles_)
        {
            size_t i = (newMax > 0) ? newMax - 1 : 0;
            for (; i < maxParticles_; i++)
            {
                particles_[i].Destroy();
            }

            liveCount_ = std::min(liveCount_, newMax);
        }

        particles_.resize(newMax);
        maxParticles_ = newMax; 
    }

    float GetSpawnRate() const { return emitter_.GetComponent<Timer>().duration; }
    void SetSpawnRate(float newRate) 
    { 
        if (newRate <= 0.0f)
        {
            LOG_ERROR_FMT("Requested spawn rate '{}' out of range. "
                "defaulting to 1.0f", newRate);
            newRate = 1.0f;
        }

        emitter_.GetComponent<Timer>().duration = newRate;
    }

    void SetRenderData(Renderable::RenderData renderData) 
    { 
        emitter_.GetComponent<Renderable>().renderData = std::move(renderData);
    }

    template <typename Fn> requires std::convertible_to<Fn, ParticleBehaviorGenerator>
    void SetBehaviorGenerator(Fn&& fn) { bxGenerator_ = std::forward<Fn>(fn); }

private:
    static bool UpdateParticleEntity(Entity& particle, float delta);

    Entity emitter_;
    std::vector<Entity> particles_;
    ParticleBehaviorGenerator bxGenerator_;
    size_t maxParticles_ = 0;
    size_t liveCount_ = 0;
};



} // test