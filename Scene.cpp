#include "Scene.h"
#include "scripting/user_types/GameControllerLuaUserTypes.h"
#include "atlas/SpriteSeriesAtlas.h"
#include "core/Logger.h"
#include "sdl/SDLite.h"
#include "sdl/SDLUtils.h"
#include "ecs/Ecs.h"
#include "physics/B2World.h"
#include "test/Premades.h"
#include "systems/RenderSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/EventCallbackSystem.h"
#include "scripting/user_types/RenderableLuaUserTypes.h"
#include "scripting/user_types/TransformLuaUserTypes.h"
#include "scripting/user_types/SpriteAnimationLuaUserTypes.h"
#include "components/GameControllerStateComponent.h"
#include "components/builder/ColliderComponentBuilder.h"
#include "components/builder/RigidBodyComponentBuilder.h"
#include "components/SpriteAnimationsComponent.h"
#include "components/driver/EventCallbackDriver.h"
#include "components/driver/ControllerInputCallbackDriver.h"
#include "test/ComponentTests.h"
#include "test/Premades.h"
#include "test/Fixtures.h"
#include "systems/CameraSystem.h"
#include "inputs/InputState.h"
#include "core/Hooks.h"
#include "core/WeightGenerator.h"
#include "core/Literals.h"
#include "core/WeightGenerator.h"
#include "physics/B2CompoundObject.h"
#include "events/Event.h"
#include "events/EventUtils.h"
#include "events/data/GameControllerEvents.h"
#include "events/data/EntityCollision.h"
#include "events/data/EntityActions.h"
#include "test/callbacks/AnimationCallbacks.h"
#include "test/callbacks/GameControllerCallbacks.h"
#include "components/driver/SpriteAnimationDriver.h"
#include "test/stateTransitions/Transitions.h"
#include "test/setups/KnightSetups.h"
#include "test/setups/SetupsUtil.h"

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
    static constexpr float kDynamicCircleRadius = 30.0f;

    static constexpr float kMaxImpulseValue = 8.0f;
    static constexpr float kImpulseScale = kMaxImpulseValue / static_cast<float>(GameController::kAxisMax);
    static constexpr float kMaxSpeed = 5.0f;

    struct Room
    {
        Entity floor, ceiling, leftWall, rightWall;
        static Result<Room> Create(B2World& world)
        {
            TRY(MakeColliderBoxEntity(world, kGroundPosition,
                kGroundCeilingDimensions, B2Body::Type::Static, {}, SDLite::kColorWhite), floorEntity);
            TRY(MakeColliderBoxEntity(world, kCeilingPosition,
                kGroundCeilingDimensions, B2Body::Type::Static, {}, SDLite::kColorWhite), ceilingEntity);
            TRY(MakeColliderBoxEntity(world, kLeftWallPosition,
                kWallDimensions, B2Body::Type::Static, {}, SDLite::kColorWhite), leftWallEntity);
            TRY(MakeColliderBoxEntity(world, kRightWallPosition,
                kWallDimensions, B2Body::Type::Static, {}, SDLite::kColorWhite), rightWallEntity);

            return Room{
                .floor = floorEntity,
                .ceiling = ceilingEntity,
                .leftWall = leftWallEntity,
                .rightWall = rightWallEntity
            };
        }
    };

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

    static constexpr std::string_view kSpritesPath = R"(resources/sprites)";

    static SpriteSeriesResourcePackets MakeKnightAtlasInfo()
    {
        static constexpr std::string_view kWalkSpriteFilePrefix = 
            R"(\knight\walk_anim\knight_walk_)";
        static constexpr size_t kNumWalkSprites = 9;

        static constexpr std::string_view kJumpSpriteFilePrefix =
            R"(\knight\jump_anim\knight_jump_)";
        static constexpr size_t kNumJumpSprites = 4;

        static constexpr std::string_view kFallSpriteFilePrefix =
            R"(\knight\fall_anim\knight_fall_)";
        static constexpr size_t kNumFallSprites = 4;

        SpriteSeriesResourcePackets knightResourcePackets{};

        auto makeSeries = [](std::string_view seriesName, std::string_view filePrefix, size_t numSprites)
        {
            static constexpr std::string_view kFilepathFmt = "resources\\sprites{}{}.png";

            ResourcePacket<SpriteSeriesMetadata> packet;
            packet.SetMetadata(SpriteSeriesMetadata{ .seriesName = std::string{seriesName} });

            std::vector<std::string> spriteFilepaths;
            spriteFilepaths.reserve(numSprites);
            for (size_t i = 0; i < numSprites; i++)
            {
                std::string filepath = std::format(kFilepathFmt, filePrefix, std::to_string(i));

                spriteFilepaths.push_back(std::move(filepath));
            }

            packet.SetFilepaths(std::move(spriteFilepaths));

            return packet;
        };

        knightResourcePackets.emplace_back(
            makeSeries(test::kWalkSeriesName, kWalkSpriteFilePrefix, kNumWalkSprites)
        );
        knightResourcePackets.emplace_back(
            makeSeries(test::kJumpSeriesName, kJumpSpriteFilePrefix, kNumJumpSprites)
        );
        knightResourcePackets.emplace_back(
            makeSeries(test::kFallSeriesName, kFallSpriteFilePrefix, kNumFallSprites)
        );

        return knightResourcePackets;
    }

    bool AxisOutsideDeadzone(SDL_FPoint axisValue)
    {
        return (std::abs(axisValue.x) > GameController::kAxisDeadzone ||
                std::abs(axisValue.y) > GameController::kAxisDeadzone);
    }
    bool AxisOutsideDeadzone(SDL_Point axisValue)
    {
        return (std::abs(axisValue.x) > GameController::kAxisDeadzone ||
            std::abs(axisValue.y) > GameController::kAxisDeadzone);
    }

    SDL_FPoint Normalize(SDL_FPoint ax)
    {
        float mag = std::sqrt(ax.x * ax.x + ax.y + ax.y);
        if (mag > 0.0001f)
        {
            return { ax.x / mag, ax.y / mag };
        }
        return { 0.0f, -1.0f };
    }

    SDL_FPoint Normalize(SDL_Point ax)
    {
        return Normalize(SDL_FPoint{ static_cast<float>(ax.x), static_cast<float>(ax.y) });
    }

    std::optional<SDL_FPoint> GetGrapplePoint(B2World& world, const GameControllerState& controller, 
                                              const RigidBody& rigidBody, float ropeLen)
    {
        auto direction = Normalize(controller.inputs[GameControllerInputSource::RightStickAxis].value.axis);
        auto playerPos = rigidBody.body.GetData().GetPosition();

        SDL_FPoint proj{
           playerPos.x + direction.x * ropeLen,
           playerPos.y + direction.y * ropeLen
        };

        auto result = world.CastRayToPoint(rigidBody.body.GetData(), proj);
        if (result.hit)
        {
            return result.point;
        }
        
        return std::nullopt;
    }

    SDL_FPoint MoveTowards(const SDL_FPoint& current, const SDL_FPoint& target, float speed) 
    {
        SDL_FPoint dir = { target.x - current.x, target.y - current.y };
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

        if (dist <= speed || dist < 0.001f) {
            return target; // Snap directly to target if close enough
        }

        float invDist = 1.0f / dist;
        SDL_FPoint normed = { dir.x * invDist, dir.y * invDist };

        return SDL_FPoint{ current.x + normed.x * speed, current.y + normed.y * speed };
    }

    enum class GrappleState
    {
        None = 0,
        Extending,
        Connected
    };

    Result<Void> HandleGrapple(B2World& world, Entity& entity, B2DistanceJoint& joint, GrappleState& state,
                               float extendSpeed, std::pair<SDL_FPoint, SDL_FPoint>& extendingPoints)
    {
        using Source = GameControllerInputSource;

        assert(entity.HasComponent<GameControllerState>());
        assert(entity.HasComponent<RigidBody>());

        auto [controller, rigidBody] = entity.GetComponents<GameControllerState, RigidBody>();
        assert(rigidBody.body.GetData().IsValid());

        if (controller.joystickID == GameController::kInvalidJoystickID)
        {
            return Void{};
        }

        /*switch (controller.buttonInput[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER].state)
        {
        case InputState::None: 
            LOG_DEBUG("None"); break;
        case InputState::Pressed:
            LOG_DEBUG("Pressed"); break;
        case InputState::Held:
            LOG_DEBUG("Held"); break;
        case InputState::Released:
            LOG_DEBUG("Released"); break;
        }*/

        if (state == GrappleState::Connected)
        {
            assert(joint.IsValid());

            if (controller.inputs[Source::B].state == InputState::Pressed)
            {
                state = GrappleState::None;
                joint.Destroy();
            }

            return Void{};
        }
      
        // either extending or none
        if (state == GrappleState::None)
        {
            if (controller.inputs[Source::RightShoulder].state == InputState::Pressed)
            {
                constexpr float grappleRopeLen = 1200.0f;

                auto grapplePoint = GetGrapplePoint(world, controller, rigidBody, grappleRopeLen);
                if (!grapplePoint.has_value())
                {
                    return Void{};
                }

                extendingPoints.first = rigidBody.body.GetData().GetPosition();
                extendingPoints.second = *grapplePoint;

                state = GrappleState::Extending;
            }
            else
            {
                return Void{};
            }                    
        }

        // extending
        assert(state == GrappleState::Extending);
        
        if (!(controller.inputs[Source::RightShoulder].state == InputState::Held ||
            controller.inputs[Source::RightShoulder].state == InputState::Pressed))
        {
            state = GrappleState::None;

            return Void{};
        }

        auto& [current, dest] = extendingPoints;

        current = MoveTowards(current, dest, extendSpeed);

        if (current.x == dest.x && current.y == dest.y)
        {
            state = GrappleState::Connected;
        }
        else
        {
            return Void{};
        }
        

        // connected
        assert(state == GrappleState::Connected);

        B2JointParams<B2DistanceJoint> params{
            .spring {.enable = true, .hertz = 8.0f, .dampingRatio = 0.7f },
            //.motor { .enable = true, .speed = -1.0f }
            .collideConnected = true
        };

        TRY_ASSIGN(joint, test::MakeGrappleJoint(world, entity, extendingPoints.second, params));

        return Void{};
    }

    void ConnectEntityToController(EventCallbackSystem& callbackSys, Entity& entity)
    {
        assert(entity.IsValid());

        using namespace events;
        //using Key = EventCallbackRegistry::Key;

        const uint32_t connectEvType = GameControllerConnected::eventType;
        const uint32_t disconnectEvType = GameControllerDisconnected::eventType;

        auto& registry = callbackSys.GetEventCallbackRegistry();

        //auto connectKey = registry.RegisterCallback("ConnectToFirstController",
        //                                             ConnectToFirstController()).key;
        //auto disconnectKey = registry.RegisterCallback("DisconnectController",
        //                                                DisconnectController()).key;

        auto& callbacks = entity.AddComponent<EventCallbacks>().table;
        //callbacks[connectEvType] = std::move(connectKey);
        //callbacks[disconnectEvType] = std::move(disconnectKey);

        entity.AddComponent(GameControllerState{});
    }

    void ApplyImpulseFromControllerInput(Entity& entity/*, bool enableJump = true*/)
    {
        assert(entity.HasComponent<GameControllerState>());
        assert(entity.HasComponent<RigidBody>());

        auto [controller, rigidBody] = entity.GetComponents<GameControllerState, RigidBody>();
        if (controller.joystickID == GameController::kInvalidJoystickID)
        {
            //LOG_WARNING("Entity joystick ID was invalid - No Impuse applied!");
            return;
        }

        auto axisValue = controller.inputs[GameControllerInputSource::RightStickAxis].value.axis;
        if (AxisOutsideDeadzone(axisValue))
        {
            axisValue *= kImpulseScale;

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
                .value = static_cast<float>(axisValue.x)
            });
        }       
    }

    //Entity MakeFPSCounterEntity(const Handle<GlyphAtlas>& atlas)
    //{
    //    auto fpsCounter = ECS::CreateEntity();
    //    assert(fpsCounter.IsValid());

    //    Renderable::Text textData{
    //        .sourceAtlas = atlas,
    //        .text = "FPS: ",
    //        .desiredDimensions = { 100, 100 },
    //        .align = Renderable::Text::Alignment::Left,
    //        .scaleToFit = false
    //    };
    //    fpsCounter.AddComponent(Renderable{
    //        .renderData = std::move(textData),
    //        .drawOrder = 100
    //    });
    //    fpsCounter.AddComponent(Transform{
    //        .position = { 100.0f, 100.0f }
    //    });

    //    return fpsCounter;
    //}

    //class FPSCounter
    //{
    //public:
    //    explicit FPSCounter(const Handle<GlyphAtlas>& atlas) : entity_(MakeFPSCounterEntity(atlas)) {}
    //    void Init(HookManager& hooks)
    //    {
    //        hooks.Attach(HookPoint::LoopStart, [this]()
    //        {
    //            frameCount_++;

    //            Uint32 currentTime = SDL_GetTicks();
    //            if (currentTime - lastTime_ < 1000)  // Update every 1000 ms = 1 second
    //            {
    //                return ReturnSignal::KeepObserving;
    //            }

    //            float fps = frameCount_ * 1000.0f / (currentTime - lastTime_);
    //            frameCount_ = 0;
    //            lastTime_ = currentTime;

    //            if (!entity_.IsValid())
    //            {
    //                return ReturnSignal::StopObserving;
    //            }
    //            if (!entity_.HasComponent<Renderable>())
    //            {
    //                return ReturnSignal::Pause;
    //            }

    //            auto& renderable = entity_.GetComponent<Renderable>();
    //            auto textData = std::get_if<Renderable::Text>(&renderable.renderData);
    //            assert(textData);

    //            textData->text = "FPS: " + std::to_string(fps);

    //            return ReturnSignal::KeepObserving;
    //        });
    //    }

    //    Uint32 lastTime_ = SDL_GetTicks();
    //    int frameCount_ = 0;
    //    Entity entity_;
    //};

    /*Entity MakeScoreboard(EventCallbackSystem& eventCallbackSystem, const Handle<GlyphAtlas>& atlas,
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

        auto handleBallCollision = 
        [player = player.GetID(), ball = ball.GetID(), ground = ground.GetID()]
        (const events::ContactCollisionBegin& ev, Entity_t scoreBoardId)
        {
            auto scoreBoard = ECS::GetEntityByID(scoreBoardId);

            assert(scoreBoard.IsValid());
            assert(scoreBoard.HasComponent<Renderable>());

            auto entA = ev.a.entity;
            auto entB = ev.b.entity;

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
                //currentScore = 0;
            }

            textData->text = std::to_string(currentScore);

            return ReturnSignal::KeepObserving;
        };
         
        auto& registry = eventCallbackSystem.GetEventCallbackRegistry();
        //auto [key, _] = registry.RegisterCallback("handleBallCollision", std::move(handleBallCollision),
        //                                          entity.GetID());

        //auto& callbacks = entity.AddComponent(EventCallbacks{}).table;
        //callbacks.AddKey(std::move(key));

        return entity;
    }*/

   
    class Chain
    {
    public:
        Chain(B2World& world, SDL_FPoint start, SDL_FPoint stop, int numLinkPoints)
        {
            SDL_FPoint dir = { stop.x - start.x, stop.y - start.y };
            float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

            float adv = dist / static_cast<float>(numLinkPoints);

            float invDist = 1.0f / dist;
            SDL_FPoint normed = { dir.x * invDist, dir.y * invDist };

            entities_.reserve(numLinkPoints + 2);
            joints_.reserve(numLinkPoints + 1);

            auto& startEnt = entities_.emplace_back(ECS::CreateEntity());
            assert(startEnt.IsValid());

            auto& startTransform = startEnt.AddComponent(Transform{});

            auto& startRigid = startEnt.AddComponent(ComponentBuilder<RigidBody>{}
            .WithBodyParameters({
                .bodyType = B2Body::Type::Dynamic,
                .position = start
            })
            .WithBodyLimits({
                .linearVelocity{ .max = { 10.0f, 10.0f } }
            })
            .Build(world));

            auto& startCollider = startEnt.AddComponent(ComponentBuilder<Collider>{}
            .WithShapeParameters({
                .shapeType = B2Shape::Type::Circle,
                .radius = 15.0f,
            })
            .WithColliderSettings({

            }).Build(startRigid.body));

            RenderProfile profile{
                .debugDraw = {
                    .boundingBox = {.on = true },
                    .collider = {.on = true }
                }
            };
            startEnt.AddComponent(Renderable{ .profile = std::move(profile) });

            auto startEntRelations = startEnt.GetRelations();
            for (int i = 0; i < numLinkPoints + 1; i++)
            {
                bool isLastPoint = (i == numLinkPoints);

                auto linkPointChild = startEntRelations.AddChild();
                assert(linkPointChild.IsValid());
                 
                SDL_FPoint linkPointPos{ 
                    start.x + normed.x * adv * (i + 1), 
                    start.y + normed.y * adv * (i + 1) 
                };

                auto& childTransform = linkPointChild.AddComponent(Transform{});

                auto& childRigid = linkPointChild.AddComponent(ComponentBuilder<RigidBody>{}
                .WithBodyParameters({
                    .bodyType = (isLastPoint ? B2Body::Type::Static : B2Body::Type::Dynamic),
                    .position = linkPointPos
                }).Build(world));

                auto& childCollider = linkPointChild.AddComponent(ComponentBuilder<Collider>{}
                .WithShapeParameters({
                    .shapeType = B2Shape::Type::Circle,
                    .radius = 5.0f
                })
                .WithColliderSettings({
                    .enableCollision = isLastPoint
                }).Build(childRigid.body));

                //Renderable::Geometry childGeo{.color = SDLite::kColorPurple };
                /*linkPointChild.AddComponent(Renderable{
                    .renderData = childGeo,
                    .drawOrder = 10,
                    .mods{ .alpha = (isLastPoint ? 255_u8 : 0_u8) }
                });*/

                assert(!entities_.empty());
                const auto prevRigid = entities_.back().GetComponent<RigidBody>();

                auto len = childRigid.body.GetData().GetDistance(prevRigid.body.GetData());

                B2JointParams<B2DistanceJoint> params{
                    .length {.rest = 0.01f, .max = len },
                    .spring {.enable = true, .hertz = 8.0f, .dampingRatio = 0.8f }
                };

                auto joint = B2JointFactory::MakeDistanceJoint(
                    prevRigid.body.GetData().GetHandle(), 
                    childRigid.body.GetData().GetHandle(), 
                    std::move(params)
                );
                assert(joint.IsValid()); 

                joints_.push_back(std::move(joint));
                entities_.push_back(std::move(linkPointChild));
            }      
        }

        std::vector<SDL_FPoint> GetPoints()
        {
            std::vector<SDL_FPoint> points;
            points.reserve(entities_.size());

            /*auto& firstJoint = joints_[0];
            auto firstJointPoints = firstJoint.GetEndPoints();
            points.push_back(firstJointPoints.first);
            points.push_back(firstJointPoints.second);

            for (int i = 1; i < joints_.size(); i++)
            {
                auto jp = joints_[i].GetEndPoints();
                points.push_back(jp.second);
            }*/

            for (auto& entity : entities_)
            {
                if (!entity.IsValid())
                {
                    LOG_ERROR("Chain entity was invalid!");
                    continue;
                }
                if (!entity.HasComponent<Transform>())
                {
                    LOG_ERROR("Chain entity did not have transform component!");
                    continue;
                }

                points.push_back(entity.GetComponent<Transform>().position);
            }

            return points;
        }

        Entity GetStartEntity() { return entities_[0]; }

    private:
        std::vector<Entity> entities_;
        std::vector<B2DistanceJoint> joints_;
    };

    class GrapplingHookDriver
    {
    public:
        enum class State
        {
            None,
            Launched,
            Connected
        };

        GrapplingHookDriver(Entity srcEntity, int numLinkPoints, B2JointParams<B2DistanceJoint> jointParams) :
            sourceEntity_(srcEntity), numLinkPoints_(numLinkPoints), jointParams_(std::move(jointParams)) {}
       
        Result<Void> Launch(B2World& world, EventCallbackSystem& callbackSystem, SDL_FPoint direction)
        {
            if (!sourceEntity_.IsValid())
            {
                return MAKE_ERROR("Source entity was invalid");
            }
            if (!sourceEntity_.HasComponent<RigidBody>())
            {
                return MAKE_ERROR("Source entity did not have a rigid body component");
            }

            auto& srcRigid = sourceEntity_.GetComponent<RigidBody>();
            srcRigid.limits.linearVelocity.max = { 0.0f, 0.0f };
            state_ = State::Launched;

            TRY(Generate(world, callbackSystem));
        }

        const Entity& GetSourceEntity() const { return sourceEntity_; }

        auto HandleSensorConnection() 
        {
            return [this](const events::SensorCollisionBegin& ev, Entity_t leadEntId) {
                auto leadEnt = ECS::GetEntityByID(leadEntId);

                if (!leadEnt.IsValid())
                {
                    LOG_WARNING("Sensor lead entity was invalid");
                    return ReturnSignal::StopObserving;
                }

                if (ev.a.entity == sourceEntity_.GetID() || ev.b.entity == sourceEntity_.GetID())
                {
                    return ReturnSignal::KeepObserving;
                }
                if (!(ev.a.entity == leadEnt.GetID() || ev.a.entity == leadEnt.GetID()))
                {
                    return ReturnSignal::KeepObserving;
                }

                assert(!entities_.empty());
                assert(leadEnt == entities_.front());

                auto currentPos = leadEnt.GetComponent<Transform>().position;

                auto& leadCollider = leadEnt.GetComponent<Collider>();
                auto& leadShape = shapeAccessor(leadCollider.shape);
                leadShape.Destroy();
                leadEnt.RemoveComponent<Collider>();

                auto& leadRigid = leadEnt.GetComponent<RigidBody>();
                auto& leadBody = bodyAccessor(leadRigid.body);
                leadBody.SetLinearVelocity({ 0.0f, 0.0f });
                leadBody.SetAngularVelocity(0.0f);
                leadBody.SetBodyType(B2Body::Type::Static);

                for (auto& joint : joints_)
                {
                    joint.SetLengthRange({ .min = 0.0f, .max = 0.00001f });
                }

                return ReturnSignal::StopObserving;
            };
        }

    private:
        Result<Void> Generate(B2World& world, EventCallbackSystem& eventCallbackSystem)
        {
            if (!sourceEntity_.IsValid())
            {
                return MAKE_ERROR("Source entity was invalid");
            }
            if (!sourceEntity_.HasComponent<RigidBody>())
            {
                return MAKE_ERROR("Source entity did not have a rigid body component");
            }
            if (!sourceEntity_.HasComponent<Collider>())
            {
                return MAKE_ERROR("Source entity did not have a collider component");
            }

            auto& srcRigid = sourceEntity_.GetComponent<RigidBody>();
            SDL_FPoint srcPos = srcRigid.body.GetData().GetPosition();
            /*auto& srcCollider = sourceEntity_.GetComponent<Collider>();
            auto srcShapeHandle = srcCollider.shape.GetData().GetHandle();*/

            std::vector<Entity> linkEntities;
            linkEntities.reserve(numLinkPoints_ + 2);

            auto& sensorLead = linkEntities.emplace_back(ECS::CreateEntity());

            sensorLead.AddComponent(Transform{});

            auto& sensorLeadRigid = sensorLead.AddComponent(ComponentBuilder<RigidBody>{}
            .WithBodyParameters({
                .bodyType = B2Body::Type::Static,
                .position = srcPos
            }).Build(world));

            sensorLead.AddComponent(ComponentBuilder<Collider>{}
            .WithShapeParameters({
                .shapeType = B2Shape::Type::Circle,
                .radius = 10.0f
            })
            .WithColliderSettings({
                .density = 5.0f,
                .enableEvents{ .sensor = true },
                .isSensor = true
                }).Build(sensorLeadRigid.body));

            auto& registry = eventCallbackSystem.GetEventCallbackRegistry();
            //auto [key, _] = registry.RegisterCallback("HandleSensorConnection", HandleSensorConnection(),
            //                                          sensorLead.GetID());

            auto& callbacks = sensorLead.AddComponent(EventCallbacks{}).table;
            //callbacks.AddKey(std::move(key));

            auto sensorLeadRelations = sensorLead.GetRelations();

            for (int i = 0; i < numLinkPoints_; i++)
            {
                auto& child = linkEntities.emplace_back(sensorLeadRelations.AddChild());

                child.AddComponent(Transform{});

                auto& childRigid = child.AddComponent(ComponentBuilder<RigidBody>{}
                .WithBodyParameters({
                    .bodyType = B2Body::Type::Dynamic,
                    .position = srcPos
                }).Build(world));

                auto& childCollider = child.AddComponent(ComponentBuilder<Collider>{}
                .WithShapeParameters({
                    .shapeType = B2Shape::Type::Circle,
                    .radius = 0.1f
                })
                .WithColliderSettings({
                    .enableCollision = false
                }).Build(childRigid.body));
            }
        }

        Entity sourceEntity_;
        int numLinkPoints_;
        B2JointParams<B2DistanceJoint> jointParams_;
        State state_ = State::None;
        std::vector<Entity> entities_;
        std::vector<B2DistanceJoint> joints_;

        static inline WriteAccessor<B2Body> bodyAccessor{};
        static inline WriteAccessor<B2Shape> shapeAccessor{};
    };
   
    void SetUpBodyTestScript(Entity& entity, std::shared_ptr<SceneFixture>& scene)
    {
        assert(entity.IsValid());
        assert(entity.HasComponent<RigidBody>());

        auto& rigid = entity.GetComponent<RigidBody>();
        auto& body = WriteAccessor<B2Body>{}(rigid.body);

        scene->SetTestScriptFile<SDL_FPoint, B2Body::Type, B2Body>(
            "b2body_test.lua", [&body](Lua& lua) { lua["body"] = &body; }
        );       
    }

    void SetUpSpriteRenderTestScript(Entity& entity, std::shared_ptr<SceneFixture>& scene)
    {
        assert(entity.IsValid());
        assert(entity.HasComponent<Transform>());
        assert(entity.HasComponent<Renderable>());
        assert(entity.HasComponent<SpriteAnimations>());

        auto& transform = entity.GetComponent<Transform>();
        auto& renderable = entity.GetComponent<Renderable>();
        auto& sprite_animations = entity.GetComponent<SpriteAnimations>();
        auto& render_profile = renderable.profile;

        assert(std::holds_alternative<SpriteRenderable>(renderable.renderData));
        auto& sprite_renderable = std::get<SpriteRenderable>(renderable.renderData);

        scene->SetTestScriptFile<
            SDL_FPoint,
            Dimensions<int>,
            SDL_Color,
            SDL_Rect,
            SDL_BlendMode,
            SDL_RendererFlip,
            TextureMods,
            DebugDraw,
            DebugDrawSet,
            RenderProfile,
            Handle<SpriteSeriesAtlas>,
            SpriteAnimationSeries,
            //SpriteAnimationSeriesMap,
            SpriteAnimations,
            Transform>("sprite_render_test.lua", [&](Lua& lua) {
                lua["transform"] = &transform;
                lua["render_profile"] = &render_profile;
                lua["sprite_renderable"] = &sprite_renderable;
                lua["sprite_animations"] = &sprite_animations;
            }
        );
    }

    void SetUpTextRenderTestScript(Entity& entity, std::shared_ptr<SceneFixture>& scene)
    {
        assert(entity.IsValid());
        assert(entity.HasComponent<Transform>());
        assert(entity.HasComponent<Renderable>());

        auto& transform = entity.GetComponent<Transform>();
        auto& renderable = entity.GetComponent<Renderable>();
        auto& render_profile = renderable.profile;
        
        assert(std::holds_alternative<TextRenderable>(renderable.renderData));
        auto& text_renderable = std::get<TextRenderable>(renderable.renderData);

        scene->SetTestScriptFile<
            SDL_FPoint, 
            Dimensions<int>,
            SDL_Color,
            SDL_Rect,
            SDL_BlendMode,
            SDL_RendererFlip,
            TextureMods,
            DebugDraw,
            DebugDrawSet,
            RenderProfile,
            TextAlign,
            Glyph,
            Handle<GlyphAtlas>,
            TextRenderable,
            Transform>("text_render_test.lua", [&](Lua& lua) { 
                lua["transform"] = &transform; 
                lua["render_profile"] = &render_profile;
                lua["text_renderable"] = &text_renderable;
            }
        );
    }

    template <SomeComponent...Ts>
    Result<Void> ValidateEntityHasComponents(const Entity& entity)
    {
        if (!entity.IsValid())
        {
            return MAKE_ERROR("Entity was invalid");
        }

        Result<Void> result{Void{}};

        auto check = [&]<typename T>() -> void {
            if (!result.Success())
            {
                return;
            }
            if (!entity.HasComponent<T>())
            {
                result = MAKE_ERROR_FMT("Entity did not component '{}'", typeid(T).name());
            }
        };

        ((check.template operator()<Ts>()), ...);

        return result;
    }

    class EntityJointChain
    {
    public:
        struct Parameters
        {
            BodyParameters bodyParams_;
            B2ShapeParameters shapeParams_;
            ColliderSettings colliderSettings_;
        };

        struct Segment
        {
            Entity entity;
            B2DistanceJoint joint;
        };

        EntityJointChain() = default;
        EntityJointChain(std::vector<Entity>&& entities, std::vector<B2DistanceJoint>&& joints) :
            entities_(std::move(entities)), joints_(std::move(joints)) {}

        B2CompoundDistanceJoint& GetJoints() { return joints_; }
        const B2CompoundDistanceJoint& GetJoints() const { return joints_; }

        std::vector<Entity>& GetEntities() { return entities_; }
        const std::vector<Entity>& GetEntities() const { return entities_; }

        Entity GetStartEntity() { return (!entities_.empty()) ? entities_.front() : Entity{}; }
        Entity GetEndEntity() { return (!entities_.empty()) ? entities_.back() : Entity{}; }

        Segment GetSegment(size_t idx)
        {
            assert(entities_.size() == joints_.GetCount());
            assert(idx < entities_.size());

            return Segment{
                .entity = entities_[idx],
                .joint = joints_[idx]
            };
        }

        std::vector<SDL_FPoint> GetPoints()
        {
            std::vector<SDL_FPoint> points;
            points.reserve(entities_.size());

            for (auto& entity : entities_)
            {
                if (!entity.IsValid())
                {
                    LOG_ERROR("Chain entity was invalid!");
                    continue;
                }
                if (!entity.HasComponent<Transform>())
                {
                    LOG_ERROR("Chain entity did not have transform component!");
                    continue;
                }

                points.push_back(entity.GetComponent<Transform>().position);
            }

            return points;
        }

        static Result<EntityJointChain> Create(B2World& world, Entity& entity, size_t numPoints, 
                                               B2JointParams<B2DistanceJoint> defaultJointParams,
                                               Parameters lastEntityParams)
        {
            TRY((ValidateEntityHasComponents<Transform, RigidBody, Collider>(entity)));

            std::vector<Entity> entities;
            std::vector<B2DistanceJoint> joints;

            entities.reserve(numPoints + 1);
            joints.reserve(numPoints + 1);

            entities.push_back(entity);

            auto& startRigid = entity.GetComponent<RigidBody>();
            if (!startRigid.body.GetData().IsValid())
            {
                return MAKE_ERROR("Start entity body was invalid");
            }

            auto& startCollider = entity.GetComponent<Collider>();
            if (!startCollider.shape.GetData().IsValid())
            {
                return MAKE_ERROR("Start entity shape was invalid");
            }

            auto groupIndex = startCollider.shape.GetData().GetCollisionFilter().groupIndex;
            if (groupIndex >= 0)
            {
                return MAKE_ERROR_FMT("Start entity must have a negative filter group index (was '{}')", 
                    groupIndex);
            }

            const SDL_FPoint startPos = startRigid.body.GetData().GetPosition();
            auto childBodyParams = BodyParameters{
                .bodyType = B2Body::Type::Dynamic,
                .position = startPos
            };
            auto childShapeParams = B2ShapeParameters{
                .shapeType = B2Shape::Type::Circle,
                .radius = 5.0f
            };
            auto childColliderSettings = ColliderSettings{
                .enableCollision = false
            };
            auto childFilter = B2CollisionFilter{
                .categories = 0,
                .categoryMask = 0,
                .groupIndex = 0
            };

            auto relations = entity.GetRelations();

            for (int i = 0; i < numPoints; i++)
            {
                if (i == numPoints - 1)
                {
                    childBodyParams = std::move(lastEntityParams.bodyParams_);
                    childShapeParams = std::move(lastEntityParams.shapeParams_);
                    childColliderSettings = std::move(lastEntityParams.colliderSettings_);
                    childFilter = B2CollisionFilter{ .groupIndex = groupIndex };
                }

                auto& child = entities.emplace_back(relations.AddChild());
                assert(child.IsValid());

                auto& prevEnt = entities.back();
                auto& prevRigid = prevEnt.GetComponent<RigidBody>();

                auto& childRigid = child.AddComponent(ComponentBuilder<RigidBody>{}
                .WithBodyParameters(childBodyParams).Build(world));

                auto& childCollider = child.AddComponent(ComponentBuilder<Collider>{}
                .WithShapeParameters(childShapeParams)
                .WithColliderSettings(childColliderSettings)
                .WithFilter(childFilter).Build(childRigid.body));

                child.AddComponent(Transform{});

                auto& joint = joints.emplace_back(
                    B2JointFactory::MakeDistanceJoint(
                        prevRigid.body.GetData().GetHandle(),
                        childRigid.body.GetData().GetHandle(),
                        defaultJointParams
                ));
                assert(joint.IsValid());
            }

            return EntityJointChain{ std::move(entities), std::move(joints) };
        }

    private:
        std::vector<Entity> entities_;
        B2CompoundDistanceJoint joints_;
    };

    Result<Void> SetUpJointChainTestScript(Entity& entity, std::shared_ptr<SceneFixture>& scene)
    {
        TRY((ValidateEntityHasComponents<RigidBody, Collider>(entity)));

        const auto pos = entity.GetComponent<RigidBody>().body.GetData().GetPosition();
        const auto noCollideGroupIdx = B2CollisionFilter::GetNextCollisionGroup(false);

        WriteAccessor<B2Shape>{}(entity.GetComponent<Collider>().shape).SetCollisionFilter({
            .groupIndex = noCollideGroupIdx
        });

        B2JointParams<B2DistanceJoint> jointParams{
            .spring {.enable = true, .hertz = 8.0f, .dampingRatio = 0.8f }
        };

        EntityJointChain::Parameters chainEndEntParams{
            .bodyParams_{
                .bodyType = B2Body::Type::Dynamic,
                .position = pos
            },
            .shapeParams_{
                .shapeType = B2Shape::Type::Circle,
                .radius = 2.0f
            },
            .colliderSettings_{
                .enableEvents{ .contact = true }
            }
        };

        TRY(EntityJointChain::Create(scene->GetWorld(), entity, 12, jointParams, chainEndEntParams),
        chain);

        /*scene->SetTestScriptFile<SDL_FPoint, B2Body::Type, B2Body>(
            "joint_chain_test.lua", [&body](Lua& lua) { lua["body"] = &body; }
        );*/
    }


    class SpriteColliderPool
    {
    public:

    private:
        std::unordered_map<std::string, Entity_t> collidersForSpriteSequences_;
    };

}

Result<Void> SimplePhysicsScene::Run()
{
    Logger::StartSession();
    SDLite::Start();

    B2World world = B2World::Create(0, 9.8f);
    
    TextureRepository textureRepo{};

    RenderSystem renderSys{}; 
    PhysicsSystem physicsSys{};
    SDLInputSystem inputSys{};

    EventCallbackSystem callbackSys{};
    callbackSys.ConnectToEventBus();

    Dimensions<float> cameraVp = { static_cast<float>(SDLite::kWindowWidth),
                                   static_cast<float>(SDLite::kWindowHeight) };
    CameraSystem cameraSys{ cameraVp };
    cameraSys.GetCamera().SetPosition(kScreenCenterPosition);

    HookManager hooks{};

    FontResourcePacket fontResource{};
    fontResource.SetMetadata(FontMetadata{
        .fontName = "default",
        .fontSize = 48,
        .fontColor = SDLite::kColorWhite
    });
    fontResource.SetFilepaths({ kFontPath });

    TRY(textureRepo.LoadNewAtlas<GlyphAtlas>(SDLite::Renderer(), std::move(fontResource)), glyphAtlasHandle);

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
    playerBody.limits.linearVelocity.max = { 15.0f, 15.0f };

    ConnectEntityToController(callbackSys, player);

    TRY(MakeColliderCircleEntity(world, kScreenCenterPosition + SDL_FPoint{ 100.0f, 0.0f }, kDynamicCircleRadius,
        B2Body::Type::Dynamic, { .restitution = 0.9f, .enableEvents{ .contact = true } }, SDLite::kColorOrange),
    ball);

    //// scoreboard
    //auto scoreboard = MakeScoreboard(callbackSys, glyphAtlasHandle, player, ball, groundEntity);
    //assert(scoreboard.IsValid());

    //// fps counter
    //auto fpsCounter = MakeFPSCounterEntity(glyphAtlasHandle);

    Uint32 lastTime = SDL_GetTicks(); 
    int frameCount = 0;

    //hooks.Attach(HookPoint::LoopStart, [&lastTime, &frameCount, entId = fpsCounter.GetID()]() 
    //{
    //    frameCount++;

    //    Uint32 currentTime = SDL_GetTicks();
    //    if (currentTime - lastTime < 1000)  // Update every 1000 ms = 1 second
    //    {
    //        return ReturnSignal::KeepObserving;
    //    }

    //    float fps = frameCount * 1000.0f / (currentTime - lastTime);
    //    frameCount = 0;
    //    lastTime = currentTime;
    //    
    //    auto fpsEnt = ECS::GetEntityByID(entId);
    //    assert(fpsEnt.IsValid());
    //    assert(fpsEnt.HasComponent<Renderable>());

    //    auto& renderable = fpsEnt.GetComponent<Renderable>();
    //    auto textData = std::get_if<Renderable::Text>(&renderable.renderData);
    //    assert(textData);

    //    textData->text = "FPS: " + std::to_string(fps);

    //    return ReturnSignal::KeepObserving;
    //});

    cameraSys.SetCameraTarget(player);

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;

    while (true)
    {
        hooks.SetHookPoint<HookPoint::LoopStart>();

        if (!inputSys.Update())
        {
            break;
        }

        ApplyImpulseFromControllerInput(player);

        physicsSys.Update(&world, timeStep, subStepCount);

        world.Step(timeStep, subStepCount);

        cameraSys.Update(GetDeltaTime());

        SDLite::Renderer().Clear(SDLite::kColorBlack);

        renderSys.Update(SDLite::Renderer(), cameraSys.GetCamera(), textureRepo);

        SDLite::Renderer().Show();
    }

    world.Destroy();

    Logger::EndSession();
    SDLite::Exit();

    return Void{};
}

Result<Void> GrapplePhysicsScene::Run()
{
    Logger::StartSession();
    SDLite::Start();

    B2World world = B2World::Create(0, 9.8f);

    TextureRepository textureRepo{};

    RenderSystem renderSys{};
    PhysicsSystem physicsSys{};
    SDLInputSystem inputSys{};
    EventCallbackSystem callbackSys{};

    Dimensions<float> cameraVp = { static_cast<float>(SDLite::kWindowWidth),
                                   static_cast<float>(SDLite::kWindowHeight) };
    CameraSystem cameraSys{ cameraVp };
    cameraSys.GetCamera().SetPosition(kScreenCenterPosition);

    HookManager hooks{};

    FontResourcePacket fontResource{};
    fontResource.SetMetadata(FontMetadata{
        .fontName = "default",
        .fontSize = 48,
        .fontColor = SDLite::kColorWhite
        });
    fontResource.SetFilepaths({ kFontPath });

    TRY(textureRepo.LoadNewAtlas<GlyphAtlas>(SDLite::Renderer(), std::move(fontResource)), glyphAtlasHandle);


    TRY(Room::Create(world), room);

    // player
    TRY(MakeColliderBoxEntity(world, kScreenCenterPosition - SDL_FPoint{ 100.0f, 0.0f }, kDynamicSquareDimensions,
        B2Body::Type::Dynamic, { .restitution = 0.7f, .enableEvents{.contact = true } }, SDLite::kColorRed),
        player);

    auto& playerRigid = player.GetComponent<RigidBody>();
    playerRigid.limits.linearVelocity.max = { 25.0f, 25.0f };

    player.AddComponent(GameControllerState{});

    ConnectEntityToController(callbackSys, player);

    TRY(MakeColliderCircleEntity(world, kScreenCenterPosition + SDL_FPoint{ 100.0f, 0.0f }, kDynamicCircleRadius,
        B2Body::Type::Dynamic, { .restitution = 0.9f, .enableEvents{.contact = true } }, SDLite::kColorOrange),
        ball);

    constexpr float extendingSpeed = 5.0f;
    std::pair<SDL_FPoint, SDL_FPoint> extendingPoints;
    B2DistanceJoint grappleJoint{};
    GrappleState grappleState = GrappleState::None;

    assert(!grappleJoint.IsValid());

    /*const auto& playerBody = playerRigid.body.GetData();
    const auto& ballBody = ball.GetComponent<RigidBody>().body.GetData();
    const float currentDist = playerBody.GetDistance(ballBody);

    B2JointParams<B2DistanceJoint> params{
        .length { .rest = currentDist, .max = currentDist },
        .spring { .enable = true, .hertz = 4.0f, .dampingRatio = 0.7f }
    };

    auto joint = B2JointFactory::MakeDistanceJoint(playerBody.GetHandle(), ballBody.GetHandle(), std::move(params));
    assert(joint.IsValid());*/

    //cameraSys.SetCameraTarget(player);

    std::optional<std::pair<SDL_FPoint, SDL_FPoint>> rope;

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;

    while (true)
    {
        hooks.SetHookPoint<HookPoint::LoopStart>();

        if (!inputSys.Update())
        {
            break;
        }

        HandleGrapple(world, player, grappleJoint, grappleState, extendingSpeed, extendingPoints);

        ApplyImpulseFromControllerInput(player);

        physicsSys.Update(&world, timeStep, subStepCount);

        world.Step(timeStep, subStepCount);

        cameraSys.Update(GetDeltaTime());

        SDLite::Renderer().Clear(SDLite::kColorBlack);

        renderSys.Update(SDLite::Renderer(), cameraSys.GetCamera(), textureRepo);

        if (grappleState == GrappleState::Extending)
        {
            assert(player.HasComponent<RigidBody>());
            const auto& body = player.GetComponent<RigidBody>().body.GetData();
            assert(body.IsValid());

            rope.emplace(body.GetPosition(), extendingPoints.first);
        }
        else if (grappleState == GrappleState::Connected)
        {
            assert(grappleJoint.IsValid());
            rope = grappleJoint.GetEndPoints();
        }
        else
        {
            rope.reset();
        }

        if (rope.has_value())
        {
            auto [p1, p2] = *rope;

            SetRenderDrawColor(SDLite::Renderer(), SDLite::kColorWhite);
            SDL_RenderDrawLineF(SDLite::Renderer(), p1.x, p1.y, p2.x, p2.y);
            SetRenderDrawColor(SDLite::Renderer(), SDLite::kColorBlack);
        }

        SDLite::Renderer().Show();
    }

    world.Destroy();

    Logger::EndSession();
    SDLite::Exit();

    return Void{};
}

Result<Void> ChainScene::Run(std::shared_ptr<SceneFixture> scene)
{
    FontResourcePacket fontResource{};
    fontResource.SetMetadata(FontMetadata{
        .fontName = "default",
        .fontSize = 48,
        .fontColor = SDLite::kColorWhite
        });
    fontResource.SetFilepaths({ kFontPath });

    TRY(scene->LoadTextureAtlas<GlyphAtlas>(std::move(fontResource)), glyphAtlasHandle);

    TRY(Room::Create(scene->GetWorld()), room);

    auto chain = Chain{
        scene->GetWorld(),
        SDL_FPoint{ kScreenCenterPosition.x, 400.0f },
        SDL_FPoint{ kScreenCenterPosition.x, 100.0f },
        20
    };

    auto chainStartEnt = chain.GetStartEntity();
    ConnectEntityToController(*scene->GetSystem<EventCallbackSystem>(), chainStartEnt);

    //SetUpBodyTestScript(chainStartEnt, scene);

    while (true)
    {
        scene->LoopStart();

        TRY(scene->UpdateSDLInputs(), cont);
        if (!cont) 
        { 
            break; 
        }

        ApplyImpulseFromControllerInput(chainStartEnt);

        TRY(scene->UpdatePhysics());

        TRY(scene->UpdateCamera());

        auto chainPoints = chain.GetPoints();

        SDLite::Renderer().Clear(SDLite::kColorBlack);

        SetRenderDrawColor(SDLite::Renderer(), SDLite::kColorWhite);

        SDL_RenderDrawLinesF(SDLite::Renderer(), chainPoints.data(), chainPoints.size());

        SetRenderDrawColor(SDLite::Renderer(), SDLite::kColorBlack);

        TRY(scene->UpdateRender());

        SDLite::Renderer().Show();

        scene->LoopEnd();
    }

    return Void{};
}

Result<Void> TextScene::Run(std::shared_ptr<SceneFixture> scene)
{
    FontResourcePacket fontResource{};
    fontResource.SetMetadata(FontMetadata{
        .fontName = "default",
        .fontSize = 48,
        .fontColor = SDLite::kColorBlack
    });
    fontResource.SetFilepaths({ kFontPath });

    TRY(scene->LoadTextureAtlas<GlyphAtlas>(std::move(fontResource)), glyphAtlasHandle);

    auto textEnt = ECS::CreateEntity();

    auto& transform = textEnt.AddComponent(Transform{
        .position = kScreenCenterPosition,
        .rotation = 0.0f,
        .scale = { 1.0f, 1.0f }
    });

    TextRenderable textRenderable{
        .sourceAtlas = glyphAtlasHandle,
        .text = "Dude he fucking turns himself into a pickle.\nFunniest shit I've ever seen",
        .dimensions = { 400, 250 },
        .align = TextAlign::Left,
        .dirtyFlags = TextRenderable::DirtyFlag::NewText
    };
    RenderProfile profile{
        
    };

    auto& renderable = textEnt.AddComponent(Renderable{
        .renderData = std::move(textRenderable),
        .profile = std::move(profile)
    });

    SetUpTextRenderTestScript(textEnt, scene);

    while (true)
    {
        scene->LoopStart();

        TRY(scene->UpdateSDLInputs(), cont);
        if (!cont)
        {
            break;
        }

        TRY(scene->UpdatePhysics());
        TRY(scene->UpdateCamera());

        SDLite::Renderer().Clear(SDLite::kColorWhite);

        TRY(scene->UpdateRender());

        SDLite::Renderer().Show();

        scene->LoopEnd();
    }

    return Void{};
}

Result<Void> SpriteScene::Run(std::shared_ptr<SceneFixture> scene)
{
    TRY(Room::Create(scene->GetWorld()), room);

    TRY(scene->LoadTextureAtlas<SpriteSeriesAtlas>(MakeKnightAtlasInfo()), 
        spriteAtlasHandle);

    const auto* spriteAtlas = scene->GetTextureRepository().GetAtlas(spriteAtlasHandle);
    assert(spriteAtlas);

    auto spriteEnt = ECS::CreateEntity();

    auto& transform = spriteEnt.AddComponent(Transform{
        .position = kScreenCenterPosition,
        .rotation = 0.0f,
        .scale = { 1.0f, 1.0f }
        });

    auto& rigidBody = spriteEnt.AddComponent(ComponentBuilder<RigidBody>{}.WithBodyParameters({
       .bodyType = B2Body::Type::Dynamic,
       .position = kScreenCenterPosition,
       .fixedRotation = true
    })
    .WithBodyLimits({
        .linearVelocity = { .max = { 15.0f, 15.0f } }
    })
    .Build(scene->GetWorld()));

    TRY(scene->GetWorld().GetBody(rigidBody.body.GetData().GetHandle()), body);
    assert(body.IsValid());

    spriteEnt.AddComponent(ComponentBuilder<Collider>{}
    .WithShapeParameters({
        .shapeType = B2Shape::Type::Polygon,
        .dimensions = Dimensions<float>{ 49.0f, 116.0f }
            //110,
            //70}
    })
    .WithColliderSettings({ 
        .friction = 15.0f,
        .enableEvents{ .contact = true } 
    })
    .Build(body));

    //SpriteRenderable spriteRenderable{
    //    .sourceAtlas = spriteAtlasHandle,
    //    .sourcePlot = spritePlots[0]
    //};
    RenderProfile profile{

    }; 

    auto& renderable = spriteEnt.AddComponent(Renderable{
        .renderData = SpriteRenderable{},
        .profile = std::move(profile)
    });

    spriteEnt.AddComponent(SpriteAnimations{});
    TRY(SpriteAnimationDriver::GetInstance(spriteEnt), spriteAnimDriver);
    spriteAnimDriver.AddSeries(test::kWalkSeriesName, *spriteAtlas);
    spriteAnimDriver.AddSeries(test::kJumpSeriesName, *spriteAtlas);

    auto& eventCallbackRegistry = scene->GetSystem<EventCallbackSystem>()->GetEventCallbackRegistry();
    //auto& transitionCallbackRegistry = scene->GetSystem<EntityStateSystem>()->GetTransitionCallbackRegistry();

    auto& controllerState = spriteEnt.AddComponent<GameControllerState>();
    auto& eventCallbacks = spriteEnt.AddComponent<EventCallbacks>();
    auto& inputCallbacks = spriteEnt.AddComponent<GameControllerInputCallbacks>();
    auto& entityStates = spriteEnt.AddComponent(EntityStateComponent{});

    using namespace events; 
    using namespace test;
    using enum GameControllerInputSource;

    auto& evTable = eventCallbacks.table;

    auto registerAndAssignEvent = [&eventCallbackRegistry, &eventCallbacks]
    (std::string_view nm, auto&& fn) {
        auto handle = eventCallbackRegistry.RegisterCallback(nm, std::move(fn));
        assert(handle.IsValid());
        eventCallbacks.table[handle.GetEventType()].push_back(handle);
    };
    auto registerAndAssignInput = [&eventCallbackRegistry, &inputCallbacks]
    (auto src, std::string_view nm, auto&& fn) {
        auto handle = eventCallbackRegistry.RegisterCallback(nm, std::move(fn));
        assert(handle.IsValid());
        inputCallbacks.table[src].push_back(handle);
    };
    //auto registerAndAssignTransition = [&transitionCallbackRegistry]
    //(auto& trans, std::string_view nm, auto&& fn) {
    //    auto view = transitionCallbackRegistry.RegisterCallback(nm, std::move(fn));
    //    assert(view.fn);
    //    trans = view;
    //};

    registerAndAssignEvent(NAME_AND_CALL(ConnectToFirstController));
    registerAndAssignEvent(NAME_AND_CALL(DisconnectController));
    registerAndAssignEvent(NAME_AND_CALL(SpriteAdvanceOnDistanceTraveled, 20));
    
    registerAndAssignInput(LeftStickAxis, NAME_AND_CALL(ApplyAxisInputToForce, kImpulseScale));

    //auto& walkState = entityStates.table[std::string{ kWalkStateName }];
    //auto& [onEnter, onExit] = walkState.transitions;
    //registerAndAssignTransition(onEnter, NAME_AND_CALL(OnWalkStateEnter));
    //registerAndAssignTransition(onExit, NAME_AND_CALL(OnWalkStateExit));
    //walkState.stateLinks = { std::string{kJumpStateName}, std::string{kFallStateName} };
    //SetUpSpriteRenderTestScript(spriteEnt, scene);

    //TRY(test::SetUpWalkResetProxyChild(spriteEnt, eventCallbackRegistry));
    TRY(test::SpinTimer(spriteEnt, eventCallbackRegistry, 2.0f,
        []() { LOG_DEBUG("TIMER FIRED!"); })
    );

    while (true)
    {
        scene->LoopStart();

        TRY(scene->UpdateSDLInputs(), cont);
        if (!cont)
        {
            break;
        }

        TRY(scene->UpdatePhysics());
        TRY(scene->UpdateCamera());

        auto vel = rigidBody.body.GetData().GetLinearVelocity();
        //LOG_DEBUG_FMT("Velocity: [{}, {}]", vel.x, vel.y);

        SDLite::Renderer().Clear(SDLite::kColorWhite);

        TRY(scene->UpdateRender());

        SDLite::Renderer().Show();

        scene->LoopEnd();
    }

    return Void{};
}