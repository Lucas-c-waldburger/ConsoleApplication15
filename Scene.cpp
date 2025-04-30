#include "Scene.h"
#include "core/Logger.h"
#include "sdl/SDLite.h"
#include "sdl/SDLUtils.h"
#include "ecs/Ecs.h"
#include "physics/B2World.h"
#include "test/Premades.h"
#include "systems/RenderSystem.h"
#include "systems/PhysicsSystem.h"
#include "events/EventSystem.h"
#include "events/custom/CustomEventDataRegistry.h"
#include "events/custom/CustomEvents.h"
#include "components/GameControllerStateComponent.h"
#include "components/EventObserverComponent.h"

namespace {
    static constexpr const char* kFontPath =
        R"(C:\Windows\WinSxS\amd64_microsoft-windows-font-truetype-arial_31bf3856ad364e35_10.0.19041.1_none_28747db34cb89a67\arial.ttf)";

    static constexpr SDL_FPoint kGroundPosition = {
        static_cast<float>(SDLite::kWindowWidth) / 2.0f,
        static_cast<float>(SDLite::kWindowHeight) - 20.0f
    };
    static constexpr SDL_FPoint kLeftWallPosition = {
        20.0f,
        static_cast<float>(SDLite::kWindowHeight) / 2.0f
    };
    static constexpr Dimensions<float> kGroundCeilingDimensions = {
         SDLite::kWindowWidth, 20.0f
    };
    static constexpr Dimensions<float> kWallDimensions = {
        20.0f, SDLite::kWindowHeight
    };
    static constexpr SDL_FPoint kRightWallPosition = {
        static_cast<float>(SDLite::kWindowWidth) - 20.0f,
        static_cast<float>(SDLite::kWindowHeight) / 2.0f
    };
    static constexpr SDL_FPoint kCeilingPosition = {
        static_cast<float>(SDLite::kWindowWidth) / 2.0f,
        20.0f
    };
    static constexpr SDL_FPoint kScreenCenterPosition = {
        static_cast<float>(SDLite::kWindowWidth) / 2.0f,
        static_cast<float>(SDLite::kWindowHeight) / 2.0f
    };
    static constexpr Dimensions<float> kDynamicSquareDimensions = {
        60.0f, 60.0f
    };

    static constexpr float kMaxImpulseValue = 8.0f;
    static constexpr float kImpuseScale = kMaxImpulseValue / static_cast<float>(GameController::kAxisMax);
    static constexpr float kMaxSpeed = 5.0f;

    Result<B2Body> AddGroundBody(B2World& world)
    {
        assert(world.IsValid());

        B2ShapeDefinition shapeDef{};
        shapeDef.shapeParams = {
            .shapeType = B2Shape::Type::Polygon,
            .dimensions = Dimensions<float>{ SDLite::kWindowWidth, 20.0f }
        };

        B2BodyDefinition bodyDef{};
        bodyDef.bodyData.type = b2_staticBody;
        bodyDef.bodyData.position = ToB2VecScaled(kGroundPosition);
       
        bodyDef.shapeDatas.push_back(std::move(shapeDef));

        return world.AddBody(bodyDef);
    }

    Result<B2Body> AddDynamicBody(B2World& world)
    {
        assert(world.IsValid());

        B2BodyDefinition bodyDef{};
        bodyDef.bodyData.type = b2_dynamicBody;
        bodyDef.bodyData.position = ToB2VecScaled(kScreenCenterPosition);
        
        return world.AddBody(bodyDef);
    }

    Result<B2Shape> AddPolyToDynamicBody(B2Body& dynamicBody)
    {
        B2ShapeDefinition shapeDef{};
        shapeDef.shapeParams = {
            .shapeType = B2Shape::Type::Polygon,
            .dimensions = Dimensions<float>{ 50.0f, 50.0f },
        };
        shapeDef.shapeDef.density = 1.0f;
        shapeDef.shapeDef.material.friction = 0.3f;
        shapeDef.shapeDef.material.restitution = 1.0f;

        return dynamicBody.AddShape(shapeDef);
    }

    ReturnSignal ConnectToFirstController(const SDL_Event& ev, Entity& ent)
    {
        assert(ev.type == GameControllerConnected::GetEventType());

        if (!ent.IsValid())
        {
            LOG_WARNING("Entity was invalid");
            return ReturnSignal::StopObserving;
        }
        if (!ent.HasComponent<GameControllerState>())
        {
            LOG_WARNING("Entity did not have GameControllerState component");
            return ReturnSignal::StopObserving;
        }

        auto& controllerState = ent.GetComponent<GameControllerState>();
        if (controllerState.joystickID != GameController::kInvalidJoystickID)
        {
            LOG_WARNING("Entity already had a joystick id marked valid");
            return ReturnSignal::Pause;
        }

        const auto* castEv = CustomEvents::GetEventData<GameControllerConnected>(ev);
        if (!castEv)
        {
            return ReturnSignal::StopObserving;
        }

        controllerState.joystickID = castEv->joystickID;
        LOG_INFO("Entity attached to new controller connection!");

        return ReturnSignal::Pause;
    };

    bool AxisOutsideDeadzone(SDL_FPoint axisValue)
    {
        return (std::abs(axisValue.x) > GameController::kAxisDeadzone ||
                std::abs(axisValue.y) > GameController::kAxisDeadzone);
    }

    void ApplyImpulseFromControllerInput(Entity& entity/*, bool enableJump = true*/)
    {
        assert(entity.HasComponent<GameControllerState>());
        assert(entity.HasComponent<RigidBody>());

        auto [controller, rigidBody] = entity.GetComponents<GameControllerState, RigidBody>();
        if (controller.joystickID == GameController::kInvalidJoystickID)
        {
            LOG_WARNING("Entity joystick ID was invalid");
            return;
        }

        auto axisValue = controller.axisInput.left.value;
        if (AxisOutsideDeadzone(axisValue))
        {
            axisValue *= kImpuseScale;

            /*bool aPressed = controller.buttonInput[SDL_CONTROLLER_BUTTON_A].state == GameControllerState::Pressed;

            if (!(aPressed && enableJump))
            {
                axisValue.y = 0.0f;                
            }
            else
            {
                axisValue.y += 60.0f;
            }*/

            rigidBody.forceRequests.impulses.push_back(Force{
                .value = axisValue
            });
        }       
    }



    Entity MakeScoreboard(const Handle<GlyphAtlas>& atlas, 
        const Entity& player, const Entity& ball, const Entity& ground)
    {
        auto entity = ECS::CreateEntity();

        Renderable::Text textData{
            .sourceAtlas = atlas,
                .text = "0",
                .desiredDimensions = { 200, 200 },
                .align = Renderable::Text::Alignment::Center
        };
        entity.AddComponent(Renderable{
            .renderData = std::move(textData),
            .drawOrder = 0
        });
        entity.AddComponent(Transform{
            .position = { SDLite::kWindowWidth / 2.0f, 150.0f }
        });

        auto& evObserver = entity.AddComponent(EventObserver{});

        evObserver.eventCallbacks[EntityCollision::ContactBegin::GetEventType()].func =
            [player = player.GetID(), ball = ball.GetID(), ground = ground.GetID()]
            (const SDL_Event& ev, Entity& scoreBoard)
            {
                assert(scoreBoard.IsValid());
                assert(scoreBoard.HasComponent<Renderable>());

                const auto* collisionEv = CustomEvents::GetEventData<EntityCollision::ContactBegin>(ev);
                assert(collisionEv);

                auto entA = collisionEv->a.entityId;
                auto entB = collisionEv->b.entityId;

                bool playerOnBall = ((entA == player && entB == ball) || 
                                     (entA == ball && entB == player));
                bool ballOnGround = ((entA == ball && entB == ground) ||
                                     (entA == ground && entB == ball));

                if (!(playerOnBall || ballOnGround))
                {
                    return ReturnSignal::KeepObserving;
                }

                auto& renderable = scoreBoard.GetComponent<Renderable>();
                auto textData = std::get_if<Renderable::Text>(&renderable.renderData);
                assert(textData);

                int currentScore = std::stoi(textData->text);
                if (playerOnBall)
                {
                    ++currentScore;
                }
                else
                {
                    currentScore = 0;
                }

                textData->text = std::to_string(currentScore);
        };

        return entity;
    }
}

Result<Void> B2Scene::Run()
{
	Logger::StartSession();
	SDLite::Start();

    B2World world = B2World::Create(0, 9.8f);

    TRY(AddGroundBody(world), groundBody);
    TRY(AddDynamicBody(world), dynamicBody);
    TRY(AddPolyToDynamicBody(dynamicBody), dynamicPolyShape);

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;
    
    SDL_Event ev;
    while (true)
    {
        SDL_FPoint forceNewtons = { 0.0 };

        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT)
            {
                break;
            }
            if (ev.type == SDL_KEYDOWN)
            {
                switch (ev.key.keysym.sym) 
                {
                case SDLK_LEFT:
                    forceNewtons.x -= 3.0f;
                    break;
                case SDLK_RIGHT:
                    forceNewtons.x += 3.0f;
                    break;
                case SDLK_UP:
                    forceNewtons.y -= 3.0f;
                    break;
                case SDLK_DOWN:
                    forceNewtons.y += 3.0f;
                    break;
                default:
                    break;
                }
            }
        }

        dynamicBody.ApplyLinearImpulse(forceNewtons, forceNewtons);

        world.Step(timeStep, subStepCount);

        SDL_FPoint dynamicPos = dynamicBody.GetPosition();
        float dynamicAngle = dynamicBody.GetAngle();

        LOG_INFO_FMT("position = [{:.2f}, {:.2f}], rotation = {:.2f}",
            dynamicPos.x, dynamicPos.y, dynamicAngle);

        SDLite::Renderer().Clear();

        assert(dynamicPolyShape.GetShapeType() == B2Shape::Type::Polygon);
        auto dynamicPolyVerts = dynamicPolyShape.GetAs<B2PolygonShape>().GetVertices();

        auto origColor = GetRenderDrawColor(SDLite::Renderer());
        SetRenderDrawColor(SDLite::Renderer(), SDLite::kColorBlack);

        SDL_RenderDrawLinesF(SDLite::Renderer(), dynamicPolyVerts.data(), dynamicPolyVerts.size());

        SetRenderDrawColor(SDLite::Renderer(), origColor);

        SDLite::Renderer().Show();
    }

    world.Destroy();

    Logger::EndSession();
    SDLite::Exit();

	return Void{};
}

Result<Void> SimplePhysicsScene::Run()
{
    Logger::StartSession();
    SDLite::Start();
    TRY((RegisterCustomEventDataTypes<TypeList<CUSTOM_EVENT_DATA_REGISTRY>>()));

    B2World world = B2World::Create(0, 9.8f);

    RenderSystem renderSys{};
    PhysicsSystem physicsSys{};
    EventSystem eventSys{};

    impl::AtlasStore store{};
    TRY(store.LoadAtlas(SDLite::Renderer(), GlyphAtlas::AtlasInfo{
        .fontPath = kFontPath, 
        .fontSize = 48, 
        .fontColor = SDLite::kColorWhite
    }), glyphAtlasHandle);

    TRY(MakeColliderBoxEntity(world, kGroundPosition, 
        kGroundCeilingDimensions, B2Body::Type::Static, {}, SDLite::kColorWhite), groundEntity);
    TRY(MakeColliderBoxEntity(world, kCeilingPosition,
        kGroundCeilingDimensions, B2Body::Type::Static, {}, SDLite::kColorWhite), ceilingEntity);
    TRY(MakeColliderBoxEntity(world, kLeftWallPosition,
        kWallDimensions, B2Body::Type::Static, {}, SDLite::kColorWhite), leftWallEntity);
    TRY(MakeColliderBoxEntity(world, kRightWallPosition,
        kWallDimensions, B2Body::Type::Static, {}, SDLite::kColorWhite), rightWallEntity);

    // player
    TRY(MakeColliderBoxEntity(world, kScreenCenterPosition - SDL_FPoint{ 100.0f, 0.0f }, kDynamicSquareDimensions,
        B2Body::Type::Dynamic, { .restitution = 0.7f, .enableEvents{ .contact = true } }, SDLite::kColorRed),
    player);

    auto& playerBody = player.GetComponent<RigidBody>();
    playerBody.limits.linearVelocity.max = { 25.0f, 25.0f };

    player.AddComponent(GameControllerState{});

    auto& evObserver = player.AddComponent(EventObserver{});
    evObserver.eventCallbacks[GameControllerConnected::GetEventType()].func = &ConnectToFirstController;

    TRY(MakeColliderBoxEntity(world, kScreenCenterPosition + SDL_FPoint{ 100.0f, 0.0f }, kDynamicSquareDimensions,
        B2Body::Type::Dynamic, { .restitution = 0.9f, .enableEvents{ .contact = true } }, SDLite::kColorOrange),
    ball);

    auto scoreboard = MakeScoreboard(glyphAtlasHandle, player, ball, groundEntity);
    assert(scoreboard.IsValid());

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;

    SDL_Event ev;
    while (true)
    {
        if (!eventSys.Poll(ev))
        {
            break;
        }

        eventSys.DistributeEvents();

        ApplyImpulseFromControllerInput(player);

        physicsSys.Update(&world, timeStep, subStepCount);

        world.Step(timeStep, subStepCount);

        SDLite::Renderer().Clear(SDLite::kColorBlack);

        renderSys.Update(SDLite::Renderer(), store);

        SDLite::Renderer().Show();
    }

    world.Destroy();

    Logger::EndSession();
    SDLite::Exit();

    return Void{};
}