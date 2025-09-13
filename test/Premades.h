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

} // test