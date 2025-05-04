#include <iostream>
#include <coroutine>
#include <variant>
#include <exception>
#include <string>
#include "sdl/SDLite.h"
#include "sdl/SDLUtils.h"
#include "systems/RenderSystem.h"
#include "scripting/ScriptManager.h"
#include "core/Monitoring.h"
#include "core/Hooks.h"
#include "systems/EventSystem.h"
#include "ecs/ECS.h"
#include "atlas/AtlasManager.h" 
#include "systems/PhysicsSystem.h"
#include "events/custom/CustomEventDataRegistry.h"
#include "test/Fixtures.h"
#include "test/Premades.h"
//#include "physics/Box.h"
#include "Scene.h"


static constexpr const char* kFontPath =
R"(C:\Windows\WinSxS\amd64_microsoft-windows-font-truetype-arial_31bf3856ad364e35_10.0.19041.1_none_28747db34cb89a67\arial.ttf)";

static constexpr std::string_view kSpritesPath = R"(resources/sprites)";
static constexpr std::string_view kWalkSeriesName = "walk";

static SpriteSeriesAtlas::AtlasInfo MakeKnightAtlasInfo()
{
    static constexpr std::string_view kWalkSpriteFilePrefix = R"(\knight\walk_anim\knight_walk_)";

    SpriteSeriesAtlas::AtlasInfo knightAtlasInfo{};
    auto& walkSeries = knightAtlasInfo.seriesDatas.emplace_back();

    walkSeries.seriesName = kWalkSeriesName;
    walkSeries.spriteFilepaths.reserve(9);
    for (int i = 0; i < 9; i++)
    {
        std::string filePath = 
            std::string{kSpritesPath} + std::string{kWalkSpriteFilePrefix} + std::to_string(i) + ".png";

        walkSeries.spriteFilepaths.push_back(std::move(filePath));
    }

    return knightAtlasInfo;
}

//static ScriptInstance MakeComponentEditScript(Spatial& spatial, Transform& tf)
//{
//    ScriptInstance instance{};
//
//    instance.scriptInfo = { 
//        .name = "component test", 
//        .scriptType = ScriptType::File, 
//        .path = "resources\\scripts\\test.lua" 
//    };
//
//    instance.setupFn = [&spatial, &tf](Lua& lua) {
//        lua["spatial"] = &spatial;
//        lua["transform"] = &tf;
//    };
//
//    return instance;
//}

static void InitGameControllerInputTest(ScriptManager& scriptManager)
{
    auto entity = ECS::CreateEntity();
    auto& controllerState = entity.AddComponent(GameControllerState{});

    ScriptInstance instance{};

    instance.scriptInfo = {
        .name = "game controller test",
        .scriptType = ScriptType::File,
        .path = "resources\\scripts\\game_controller_test.lua"
    };

    instance.setupFn = [&controllerState](Lua& lua) {
        lua["gameControllerState"] = &controllerState;
    };

    scriptManager.RegisterScript<
        SDL_FPoint,
        SDL_GameControllerButton,
        HandedPair<AxisInputData>,
        AxisInputData,
        ButtonInputData,
        GameControllerState>(std::move(instance));
}

static void SetKnightControllerConnectedCallback(EventObserver& knightEvents)
{
    knightEvents.eventCallbacks[GameControllerConnected::GetEventType()].func =
    [](const SDL_Event& ev, Entity& self) -> ReturnSignal 
    {
        assert(ev.type == GameControllerConnected::GetEventType());

        if (!self.IsValid())
        {
            LOG_WARNING("Entity was invalid");
            return ReturnSignal::StopObserving;
        }
        if (!self.HasComponent<GameControllerState>())
        {
            LOG_WARNING("Entity did not have GameControllerState component");
            return ReturnSignal::StopObserving;
        }

        auto& controllerState = self.GetComponent<GameControllerState>();
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
}

static void UpdateControllerForce(Entity& entity)
{
    //assert((entity.HasComponents<RigidBody, GameControllerState>()));

    //auto [rigidBody, controller] = entity.GetComponents<RigidBody, GameControllerState>();

    //const auto [leftX, leftY] = controller.axisInput.left.value;

    //auto getNormedVal = [max = phys.forces.max](const auto xOrY) -> float {
    //    return (std::abs(xOrY) > GameController::kAxisDeadzone) ?
    //        xOrY / static_cast<float>(GameController::kAxisMax) * max : 0.0f;
    //};

    //SDL_FPoint normed = {
    //    .x = getNormedVal(leftX),
    //    .y = getNormedVal(leftY)
    //};

    //phys.forces.normed.push_back(Force{ .vector = normed, .duration = 0 });
}

int main(int argc, char* argv[]) 
{
    /*
    Logger::StartSession();
    SDLite::Start();
    ASSERT_RESULT(RegisterCustomEventDataTypes<TypeList<CUSTOM_EVENT_DATA_REGISTRY>>());
      
    Entity knight = ECS::CreateEntity();

    impl::AtlasStore atlasStore{};
    auto spriteHandleResult = atlasStore.LoadAtlas(SDLite::Renderer(), MakeKnightAtlasInfo());
    assert(spriteHandleResult.Success());

    auto spriteAtlas = atlasStore.GetAtlas(*spriteHandleResult);
    assert(spriteAtlas);

    SDL_Rect spriteRect = spriteAtlas->GetSprite(kWalkSeriesName, 0).atlasRect;

    auto& knightRenderable = knight.AddComponent(Renderable{
        .renderData = Renderable::Sprite{
            .sourceAtlas = *spriteHandleResult,
            .seriesName = std::string{kWalkSeriesName},
            .currentIndex = 0
        },
        .drawOrder = 0  
        }
    );
    auto& knightSpatial = knight.AddComponent(Spatial{
        .position = SDL_FPoint{ SDLite::kWindowWidth / 2.0f, SDLite::kWindowHeight / 2.0f },
        .dimensions = {static_cast<float>(spriteRect.w), static_cast<float>(spriteRect.h)}
    });
    auto& knightTransform = knight.AddComponent(Transform{});
    auto& knightPhysics = knight.AddComponent(Physics{
        .mass = 5.0f,
        .forces{ .max = 50.0f }
    });
    auto& knightCollision = knight.AddComponent(Collider{
        .position = knightSpatial.position,
        .dimensions = knightSpatial.dimensions,
        .profile = (Collider::ApplyScale | Collider::Solid | Collider::Dynamic)
    });
    knightCollision.material.restitution = 0.1f;
    auto& knightControllerState = knight.AddComponent(GameControllerState{});
    auto& knightEvents = knight.AddComponent(EventObserver{});

    SetKnightControllerConnectedCallback(knightEvents);
    */


    ////
    //auto entB = ECS::CreateEntity();

    //auto textHandleResult = renderSys.LoadAtlas(SDLite::Renderer(), handleManager,
    //    GlyphAtlas::AtlasInfo{.fontPath = kFontPath, .fontSize = 48, .fontColor = { 0, 0, 0, 255 }});

    //assert(textHandleResult.Success());

    //entB.AddComponent(Renderable{
    //    .renderData = Renderable::Text{
    //        .sourceAtlas = *textHandleResult,
    //        .text = "I'm a cute bug\nwith a big sword",
    //        .align = Renderable::Text::Alignment::Left,
    //        .scaleToFit = false
    //    },
    //    .drawOrder = 0
    //    }
    //);
    //auto& bSpatial = entB.AddComponent(Spatial{
    //    .position = SDL_FPoint{ SDLite::kWindowWidth / 2.0f, SDLite::kWindowHeight / 2.0f - 200.0f },
    //    .dimensions = { 600, 300 }
    //    }
    //);
    //auto& bTf = entB.AddComponent(Transform{});

    // LUA //
    //auto lua = Lua::GetInstance<SDL_FPoint, Dimensions<float>, Spatial, Transform>();
    //
    //lua.SetScript({ .name = "scripts\\test.lua", .scriptType = Lua::ScriptType::File });
    //
    //lua["spatial"] = &spatialA;
    //lua["transform"] = &tfA;
    //
    //ASSERT_RESULT(lua.Run());
    //
    //FileMonitor fileMonitor{ "scripts\\test.lua" }; 
    //
    //fileMonitor.AddObserver([&lua]() {
    //    auto runResult = lua.Run();
    //    if (!runResult.Success())
    //    {
    //        std::cerr << runResult.GetError();
    //        return ReturnSignal::StopObserving;
    //    }
    //    return ReturnSignal::KeepObserving;
    //});
    //
    //fileMonitor.Start();



    /*
    RenderSystem renderSys{};
    PhysicsSystem physSystem{};
    CollisionSystem colSystem{};
    //ScriptManager scriptManager{};
    EventSystem eventSystem{};

    InitSimpleEnvironment(colSystem);
    */


    //RunBox2DSample();

    //auto physFixture = ScriptFixture::GetInstance::PhysicsEditor(knight);
    //assert(physFixture);

    //scriptManager.RegisterScript<SDL_FPoint, Dimensions<float>, Spatial, Transform>(
    //    MakeComponentEditScript(knightSpatial, knightTransform)
    //);
    //InitGameControllerInputTest(scriptManager);

    //FileChangeMonitor fileMonitor{ "resources\\scripts\\game_controller_test.lua" };

    /*
    SDL_Event ev;
    while (true)
    {
        if (!eventSystem.Poll(ev))
        {
            break;
        }

        eventSystem.DistributeEvents();

        SDLite::Renderer().Clear();

        UpdateControllerForce(knight);

        Hooks::SetHookPoint(HookPoint::PrePhysicsUpdate);

        physSystem.Update(colSystem, static_cast<float>(GetDeltaTime()));

        Hooks::SetHookPoint(HookPoint::PostPhysicsUpdate);

        renderSys.Update(SDLite::Renderer(), atlasStore);

        SDLite::Renderer().Show();
    }

    Logger::EndSession();

    SDLite::Exit();
    */


    ASSERT_RESULT(SimplePhysicsScene::Run());

    return 0;
}

 