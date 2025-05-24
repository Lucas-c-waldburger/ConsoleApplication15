#include "Scene.h"
#include "core/Logger.h"
#include "sdl/SDLite.h"
#include "sdl/SDLUtils.h"
#include "ecs/Ecs.h"
#include "physics/B2World.h"
#include "test/Premades.h"
#include "systems/RenderSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/EventSystem.h"
#include "events/custom/CustomEventDataRegistry.h"
#include "events/custom/CustomEvents.h"
#include "components/GameControllerStateComponent.h"
#include "components/EventObserverComponent.h"
#include "test/ComponentTests.h"
#include "test/Fixtures.h"
#include "systems/CameraSystem.h"
#include "inputs/InputState.h"
#include "core/Hooks.h"
#include "core/WeightGenerator.h"
#include "core/Literals.h"
#include "core/WeightGenerator.h"
#include "physics/B2CompoundObject.h"

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
    static constexpr float kImpuseScale = kMaxImpulseValue / static_cast<float>(GameController::kAxisMax);
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

    ReturnSignal InvalidateEntityJoystickIDAndListenForNewConnection(const SDL_Event& ev, Entity& ent)
    {
        assert(ev.type == GameControllerDisconnected::GetEventType());

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

        // 1. mark entity's joystickID as invalid in its controller state
        const auto* disconnectEv = CustomEvents::GetEventData<GameControllerDisconnected>(ev);
        assert(disconnectEv);

        auto& controllerState = ent.GetComponent<GameControllerState>();
        if (controllerState.joystickID != disconnectEv->joystickID)
        {
            LOG_DEBUG("Entity's connected controller different from the one that was disconnected");
            return ReturnSignal::KeepObserving;
        }

        controllerState.joystickID = GameController::kInvalidJoystickID;

        LOG_DEBUG("Set entity's controller state joystickID to invalid");

        // 2. unpause listening for new controller connection
        assert(ent.HasComponent<EventObserver>());
        auto& entityEvents = ent.GetComponent<EventObserver>();
       
        auto connectEvIt = entityEvents.eventCallbacks.find(GameControllerConnected::GetEventType());

        assert(connectEvIt != entityEvents.eventCallbacks.end());
        assert(connectEvIt->second.status == ReturnSignal::Pause);

        connectEvIt->second.status = ReturnSignal::KeepObserving;

        LOG_DEBUG("Listening for a new connection on this entity...");

        return ReturnSignal::KeepObserving;
    }

    bool AxisOutsideDeadzone(SDL_FPoint axisValue)
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

    std::optional<SDL_FPoint> GetGrapplePoint(B2World& world, const GameControllerState& controller, 
                                              const RigidBody& rigidBody, float ropeLen)
    {
        auto direction = Normalize(controller.axisInput.right.value);
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


    /*SDL_FPoint GetGrapplePoint(const GameControllerState& controller, const RigidBody& rigidBody, float ropeLen)
    {
        auto direction = NormalizeAxis(controller.axisInput.right.value);
        auto playerPos = rigidBody.body.GetData().GetPosition();

        return {
           playerPos.x + direction.x * ropeLen,
           playerPos.y + direction.y * ropeLen
        };
    }*/

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

            if (controller.buttonInput[SDL_CONTROLLER_BUTTON_B].state == InputState::Pressed)
            {
                state = GrappleState::None;
                joint.Destroy();
            }

            return Void{};
        }
      
        // either extending or none
        if (state == GrappleState::None)
        {
            if (controller.buttonInput[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER].state == InputState::Pressed)
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
        
        if (!(controller.buttonInput[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER].state == InputState::Held ||
            controller.buttonInput[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER].state == InputState::Pressed))
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

    void ConnectEntityToController(Entity& entity)
    {
        assert(entity.IsValid());

        entity.AddComponent(GameControllerState{});

        auto& evs = entity.AddComponent(EventObserver{});

        evs.eventCallbacks[GameControllerConnected::GetEventType()].func =
            &ConnectToFirstController;
        evs.eventCallbacks[GameControllerDisconnected::GetEventType()].func =
            &InvalidateEntityJoystickIDAndListenForNewConnection;
    }

    void ApplyImpulseFromControllerInput(Entity& entity/*, bool enableJump = true*/)
    {
        assert(entity.HasComponent<GameControllerState>());
        assert(entity.HasComponent<RigidBody>());

        auto [controller, rigidBody] = entity.GetComponents<GameControllerState, RigidBody>();
        if (controller.joystickID == GameController::kInvalidJoystickID)
        {
            LOG_WARNING("Entity joystick ID was invalid - No Impuse applied!");
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

    Entity MakeFPSCounterEntity(const Handle<GlyphAtlas>& atlas)
    {
        auto fpsCounter = ECS::CreateEntity();
        assert(fpsCounter.IsValid());

        Renderable::Text textData{
            .sourceAtlas = atlas,
            .text = "FPS: ",
            .desiredDimensions = { 100, 100 },
            .align = Renderable::Text::Alignment::Left,
            .scaleToFit = false
        };
        fpsCounter.AddComponent(Renderable{
            .renderData = std::move(textData),
            .drawOrder = 100
        });
        fpsCounter.AddComponent(Transform{
            .position = { 100.0f, 100.0f }
        });

        return fpsCounter;
    }

    class FPSCounter
    {
    public:
        explicit FPSCounter(const Handle<GlyphAtlas>& atlas) : entity_(MakeFPSCounterEntity(atlas)) {}
        void Init(HookManager& hooks)
        {
            hooks.Attach(HookPoint::LoopStart, [this]()
            {
                frameCount_++;

                Uint32 currentTime = SDL_GetTicks();
                if (currentTime - lastTime_ < 1000)  // Update every 1000 ms = 1 second
                {
                    return ReturnSignal::KeepObserving;
                }

                float fps = frameCount_ * 1000.0f / (currentTime - lastTime_);
                frameCount_ = 0;
                lastTime_ = currentTime;

                if (!entity_.IsValid())
                {
                    return ReturnSignal::StopObserving;
                }
                if (!entity_.HasComponent<Renderable>())
                {
                    return ReturnSignal::Pause;
                }

                auto& renderable = entity_.GetComponent<Renderable>();
                auto textData = std::get_if<Renderable::Text>(&renderable.renderData);
                assert(textData);

                textData->text = "FPS: " + std::to_string(fps);

                return ReturnSignal::KeepObserving;
            });
        }

        Uint32 lastTime_ = SDL_GetTicks();
        int frameCount_ = 0;
        Entity entity_;
    };

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
                    //currentScore = 0;
                }

                textData->text = std::to_string(currentScore);
        };

        return entity;
    }

    class Counter
    {
    public:
        double GetDeltaTime() const
        {
            return delta_;
        }

        void Refresh()
        {
            uint64_t now = SDL_GetPerformanceCounter();

            delta_ = static_cast<double>(now - last_) /
                static_cast<double>(SDL_GetPerformanceFrequency());

            last_ = now;
        }

        void Init(HookManager& hooks)
        {
            hooks.Attach(HookPoint::LoopStart, [this]() {
                this->Refresh();
                return ReturnSignal::KeepObserving;
            });
        }

    private:
        uint64_t last_ = 0;
        double delta_ = 0.0;
    };

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

            Renderable::Geometry startGeo{ .color = SDLite::kColorOrange };
            startEnt.AddComponent(Renderable{
                .renderData = startGeo,
                .drawOrder = 10
            });

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
                    .length {.rest = 0.0001f, .max = len },
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
       
        Result<Void> Launch(B2World& world, SDL_FPoint direction)
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

            TRY(Generate(world));
        }

        const Entity& GetSourceEntity() const { return sourceEntity_; }

        ReturnSignal HandleSensorConnection(const SDL_Event& ev, Entity& leadEnt) 
        {
            if (!leadEnt.IsValid())
            {
                LOG_WARNING("Sensor lead entity was invalid");
                return ReturnSignal::StopObserving;
            }

            const auto* sensorBegEv = CustomEvents::GetEventData<EntityCollision::SensorBegin>(ev);
            assert(sensorBegEv);

            const auto& [a, b] = *sensorBegEv;
            if (a.entityId == sourceEntity_.GetID() || b.entityId == sourceEntity_.GetID())
            {
                return ReturnSignal::KeepObserving;
            }
            if (!(a.entityId == leadEnt.GetID() || b.entityId == leadEnt.GetID()))
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

    private:
        Result<Void> Generate(B2World& world)
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

            auto& sensorLeadEvs = sensorLead.AddComponent(EventObserver{});
            auto& cb = sensorLeadEvs.eventCallbacks[EntityCollision::SensorBegin::GetEventType()];
            cb.func = [this](const SDL_Event& ev, Entity& sensorLeadEnt) {
                return this->HandleSensorConnection(ev, sensorLeadEnt);
            };
            
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

        //static SDL_FPoint GetDestinationPoint(SDL_FPoint direction, float len)
        //{

        //}

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

    template <ComponentType...Ts>
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

}

//Result<Void> B2Scene::Run()
//{
//	Logger::StartSession();
//	SDLite::Start();
//
//    B2World world = B2World::Create(0, 9.8f);
//
//    TRY(AddGroundBody(world), groundBody);
//    TRY(AddDynamicBody(world), dynamicBody);
//    TRY(AddPolyToDynamicBody(dynamicBody), dynamicPolyShape);
//
//    float timeStep = 1.0f / 60.0f;
//    int subStepCount = 4;
//    
//    SDL_Event ev;
//    while (true)
//    {
//        SDL_FPoint forceNewtons = { 0.0 };
//
//        while (SDL_PollEvent(&ev))
//        {
//            if (ev.type == SDL_QUIT)
//            {
//                break;
//            }
//            if (ev.type == SDL_KEYDOWN)
//            {
//                switch (ev.key.keysym.sym) 
//                {
//                case SDLK_LEFT:
//                    forceNewtons.x -= 3.0f;
//                    break;
//                case SDLK_RIGHT:
//                    forceNewtons.x += 3.0f;
//                    break;
//                case SDLK_UP:
//                    forceNewtons.y -= 3.0f;
//                    break;
//                case SDLK_DOWN:
//                    forceNewtons.y += 3.0f;
//                    break;
//                default:
//                    break;
//                }
//            }
//        }
//
//        dynamicBody.ApplyLinearImpulse(forceNewtons, forceNewtons);
//
//        world.Step(timeStep, subStepCount);
//
//        SDL_FPoint dynamicPos = dynamicBody.GetPosition();
//        float dynamicAngle = dynamicBody.GetAngle();
//
//        LOG_INFO_FMT("position = [{:.2f}, {:.2f}], rotation = {:.2f}",
//            dynamicPos.x, dynamicPos.y, dynamicAngle);
//
//        SDLite::Renderer().Clear();
//
//        assert(dynamicPolyShape.GetShapeType() == B2Shape::Type::Polygon);
//        auto dynamicPolyVerts = dynamicPolyShape.GetAs<B2PolygonShape>().GetVertices();
//
//        auto origColor = GetRenderDrawColor(SDLite::Renderer());
//        SetRenderDrawColor(SDLite::Renderer(), SDLite::kColorBlack);
//
//        SDL_RenderDrawLinesF(SDLite::Renderer(), dynamicPolyVerts.data(), dynamicPolyVerts.size());
//
//        SetRenderDrawColor(SDLite::Renderer(), origColor);
//
//        SDLite::Renderer().Show();
//    }
//
//    world.Destroy();
//
//    Logger::EndSession();
//    SDLite::Exit();
//
//	return Void{};
//}

Result<Void> SimplePhysicsScene::Run()
{
    Logger::StartSession();
    SDLite::Start();
    TRY((RegisterCustomEventDataTypes<CUSTOM_EVENT_DATA_REGISTRY>()));

    B2World world = B2World::Create(0, 9.8f);

    RenderSystem renderSys{}; 
    PhysicsSystem physicsSys{};
    EventSystem eventSys{};

    Dimensions<float> cameraVp = { static_cast<float>(SDLite::kWindowWidth),
                                   static_cast<float>(SDLite::kWindowHeight) };
    CameraSystem cameraSys{ cameraVp };
    cameraSys.GetCamera().SetPosition(kScreenCenterPosition);

    HookManager hooks{};

    impl::TextureManager store{};
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

    auto& playerEvents = player.AddComponent(EventObserver{});
    playerEvents.eventCallbacks[GameControllerConnected::GetEventType()].func = 
        &ConnectToFirstController;
    playerEvents.eventCallbacks[GameControllerDisconnected::GetEventType()].func = 
        &InvalidateEntityJoystickIDAndListenForNewConnection;

    TRY(MakeColliderCircleEntity(world, kScreenCenterPosition + SDL_FPoint{ 100.0f, 0.0f }, kDynamicCircleRadius,
        B2Body::Type::Dynamic, { .restitution = 0.9f, .enableEvents{ .contact = true } }, SDLite::kColorOrange),
    ball);

    // scoreboard
    auto scoreboard = MakeScoreboard(glyphAtlasHandle, player, ball, groundEntity);
    assert(scoreboard.IsValid());

    // fps counter
    auto fpsCounter = MakeFPSCounterEntity(glyphAtlasHandle);

    Uint32 lastTime = SDL_GetTicks(); 
    int frameCount = 0;

    hooks.Attach(HookPoint::LoopStart, [&lastTime, &frameCount, entId = fpsCounter.GetID()]() 
    {
        frameCount++;

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastTime < 1000)  // Update every 1000 ms = 1 second
        {
            return ReturnSignal::KeepObserving;
        }

        float fps = frameCount * 1000.0f / (currentTime - lastTime);
        frameCount = 0;
        lastTime = currentTime;
        
        auto fpsEnt = ECS::GetEntityByID(entId);
        assert(fpsEnt.IsValid());
        assert(fpsEnt.HasComponent<Renderable>());

        auto& renderable = fpsEnt.GetComponent<Renderable>();
        auto textData = std::get_if<Renderable::Text>(&renderable.renderData);
        assert(textData);

        textData->text = "FPS: " + std::to_string(fps);

        return ReturnSignal::KeepObserving;
    });

    cameraSys.SetCameraTarget(player);

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;

    SDL_Event ev;
    while (true)
    {
        hooks.SetHookPoint<HookPoint::LoopStart>();

        if (!eventSys.Poll(ev))
        {
            break;
        }

        eventSys.DistributeEvents();

        ApplyImpulseFromControllerInput(player);

        physicsSys.Update(&world, timeStep, subStepCount);

        world.Step(timeStep, subStepCount);

        cameraSys.Update(GetDeltaTime());

        SDLite::Renderer().Clear(SDLite::kColorBlack);

        renderSys.Update(SDLite::Renderer(), cameraSys.GetCamera(), store);

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
    TRY((RegisterCustomEventDataTypes<CUSTOM_EVENT_DATA_REGISTRY>()));

    B2World world = B2World::Create(0, 9.8f);

    RenderSystem renderSys{};
    PhysicsSystem physicsSys{};
    EventSystem eventSys{};

    Dimensions<float> cameraVp = { static_cast<float>(SDLite::kWindowWidth),
                                   static_cast<float>(SDLite::kWindowHeight) };
    CameraSystem cameraSys{ cameraVp };
    cameraSys.GetCamera().SetPosition(kScreenCenterPosition);

    HookManager hooks{};

    impl::TextureManager store{};
    TRY(store.LoadAtlas(SDLite::Renderer(), GlyphAtlas::AtlasInfo{
        .fontPath = kFontPath,
            .fontSize = 48,
            .fontColor = SDLite::kColorWhite
    }), glyphAtlasHandle);

    TRY(Room::Create(world), room);

    // player
    TRY(MakeColliderBoxEntity(world, kScreenCenterPosition - SDL_FPoint{ 100.0f, 0.0f }, kDynamicSquareDimensions,
        B2Body::Type::Dynamic, { .restitution = 0.7f, .enableEvents{.contact = true } }, SDLite::kColorRed),
        player);

    auto& playerRigid = player.GetComponent<RigidBody>();
    playerRigid.limits.linearVelocity.max = { 25.0f, 25.0f };

    player.AddComponent(GameControllerState{});

    auto& playerEvents = player.AddComponent(EventObserver{});
    playerEvents.eventCallbacks[GameControllerConnected::GetEventType()].func =
        &ConnectToFirstController;
    playerEvents.eventCallbacks[GameControllerDisconnected::GetEventType()].func =
        &InvalidateEntityJoystickIDAndListenForNewConnection;

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

    SDL_Event ev;
    while (true)
    {
        hooks.SetHookPoint<HookPoint::LoopStart>();

        if (!eventSys.Poll(ev))
        {
            break;
        }

        eventSys.DistributeEvents();

        HandleGrapple(world, player, grappleJoint, grappleState, extendingSpeed, extendingPoints);

        ApplyImpulseFromControllerInput(player);

        physicsSys.Update(&world, timeStep, subStepCount);

        world.Step(timeStep, subStepCount);

        cameraSys.Update(GetDeltaTime());

        SDLite::Renderer().Clear(SDLite::kColorBlack);

        renderSys.Update(SDLite::Renderer(), cameraSys.GetCamera(), store);

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
    TRY(scene->LoadTextureAtlas<GlyphAtlas>({
        .fontPath = kFontPath,
        .fontSize = 24,
        .fontColor = SDLite::kColorWhite
    }), glyphAtlasHandle);

    TRY(Room::Create(scene->GetWorld()), room);

    auto chain = Chain{
        scene->GetWorld(),
        SDL_FPoint{ kScreenCenterPosition.x, 400.0f },
        SDL_FPoint{ kScreenCenterPosition.x, 100.0f },
        20
    };

    auto chainStartEnt = chain.GetStartEntity();
    ConnectEntityToController(chainStartEnt);

    SetUpBodyTestScript(chainStartEnt, scene);

    while (true)
    {
        scene->LoopStart();

        TRY(scene->UpdateEvents(), cont);
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
    }

    return Void{};
}