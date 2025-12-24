#include <iostream>
#include <string>
#include "file/FilePathUtility.h"
#include "test/catch/Config.h"
#include "deps/catch/catch_amalgamated.hpp"
#include "test/Fixtures.h"
#include "test/ui/EntityView.h"

int main(int argc, char* argv[]) 
{
    FilePathUtility::Init(argv[0]);

#if RUN_MODE(RUN_CATCH_TESTS)

    return Catch::Session().run(argc, argv);

#elif RUN_MODE(RUN_UI_VISUALIZER)
    auto fixtureResult = SceneFixture::GetInstance();
    ASSERT_RESULT(fixtureResult);
    
    auto& fixture = fixtureResult.GetValue();


    auto spriteAtlasResult = fixture->GetTextureRepository()
        .CreateAtlas<SpriteAtlas>(fixture->GetRenderer());
    ASSERT_RESULT(spriteAtlasResult);
    auto& spriteAtlas = spriteAtlasResult.GetValue();

    auto pathResult = ResourcePath::Sprite("environment/ground_dirt_0.png");
    ASSERT_RESULT(pathResult);
    auto spriteResult = spriteAtlas->LoadSprite(fixture->GetRenderer(),
        { .filepath = std::move(pathResult.GetValue()) }
    );
    ASSERT_RESULT(spriteResult);
    assert(spriteResult.GetValue().sourceAtlas.IsValid());
    auto floorSprite = spriteResult.GetValue();

    auto floor = ECS::CreateEntity();

    floor.AddComponent(Transform{});
    auto& renderable = floor.AddComponent(SpriteRenderableComponent{});
    renderable.profile.debugDraw.collider.on = true;

    auto floorRigid = floor.AddComponent(ComponentBuilder<RigidBody>{}
    .WithBodyParameters({
        .bodyType = B2Body::Type::Static,
        .position = {
            .x = SDLite::Window().GetLocalCenter<SDL_FPoint>().x,
            .y = SDLite::Window().GetSize<float>().h - 48.0f
        }
    }).Build(fixture->GetWorld()));

    assert(floorRigid.body.GetData().IsValid());

    auto& floorCollider = floor.AddComponent(ComponentBuilder<Collider>{}
    .WithShapeParameters({
        .shapeType = B2Shape::Type::Polygon,
        .dimensions = Dimensions{
            .w = 96.0f * 20.0f,
            .h = 96.0f
        }
    })
    .WithColliderSettings({
        .enableEvents = { .contact = true }
    }).Build(WriteAccessor<B2Body>{}(floorRigid.body)));

    assert(floorCollider.shape.GetData().IsValid());

    auto floorBbox = floorCollider.shape.GetData().GetBoundingBox();
    assert(floorBbox.x < 0.0f);

    float xPos = floorBbox.x + 48.0f;
    float yPos = SDLite::Window().GetSize<float>().h - 48.0f;
    auto floorRels = floor.GetRelations();
    for (size_t i = 0; i < 20; i++)
    {
        auto child = floorRels.AddChild();
        child.AddComponent<Transform>().position = { xPos, yPos };
        child.AddComponent<SpriteRenderableComponent>().sprite = floorSprite;

        xPos += 96.0f;
    }

    /*auto ent = ECS::CreateEntity();

    ent.AddComponent(Transform{
        .position = SDLite::Window().GetLocalCenter<SDL_FPoint>()
    });
    ent.AddComponent(SpriteRenderableComponent{ 
        .sprite = spriteResult.GetValue(),
        .profile = { 
            .debugDraw = { 
                .boundingBox = { .on = true }, 
                .collider = { .on = true }
            }
        }
    });

    auto entRigid = ent.AddComponent(ComponentBuilder<RigidBody>{}
    .WithBodyParameters({
        .bodyType = B2Body::Type::Dynamic,
        .position = SDLite::Window().GetLocalCenter<SDL_FPoint>()
    }).Build(fixture->GetWorld()));

    assert(entRigid.body.GetData().IsValid());

    auto& entCollider = ent.AddComponent(ComponentBuilder<Collider>{}
    .WithShapeParameters({
        .shapeType = B2Shape::Type::Polygon,
        .dimensions = Dimensions{ 45.0f, 120.0f },
        .localPosition = SDL_FPoint{ 0.0f, 26.0f }
    })
    .WithColliderSettings({
        .enableEvents = {.contact = true }
    }).Build(WriteAccessor<B2Body>{}(entRigid.body)));

    assert(entCollider.shape.GetData().IsValid());

    ASSERT_RESULT(fixture->RunGameLoop());*/

    auto entity = ECS::CreateEntity();

    //auto& tf = 
        //entity.AddComponent(Transform{ .position = { 20.0f, 34.0f }, .rotation = 24.0f });

    auto& gui = fixture->GetSystem<GuiSystem>();

    ASSERT_RESULT(ui::EntityInspector::Init(fixture));
    bool cont = true;
    //std::string label = "Transform";

    gui->AddWidget("Entity Inspector", [&] {
        cont = ui::EntityInspector::Draw(entity);
        //DrawComponentEditor(tf);
    });

    ASSERT_RESULT(fixture->RunGameLoop());


    //ImGuiIO& io = ImGui::GetIO();
    //ImGuiStyle& style = ImGui::GetStyle();

    //io.FontGlobalScale = 1.5f;
    //style.ScaleAllSizes(1.3f);

    //static constexpr std::array kComponentNames = {
    //    "Transform", "SpriteRenderable", "TextRenderable"
    //};
    //size_t selectedIndex = 0;
    //float x = 0.0f;

    //gui->AddWidget("Components", [&] {
    //    for (size_t i = 0; i < kComponentNames.size(); i++)
    //    {
    //        const bool isSelected = (selectedIndex == i);

    //        if (ImGui::Selectable(kComponentNames[i], isSelected))
    //        {
    //            selectedIndex = i; // clicked
    //        }

    //        ImGui::SameLine(ImGui::GetWindowWidth() - 30);  // Push to right side

    //        ImGui::PushID((int)i);  // To avoid ID collisions
    //        if (ImGui::SmallButton("X"))
    //        {
    //            // Remove component
    //            //components.erase(components.begin() + i);

    //            // Fix selection
    //            //if (selectedIndex >= components.size())
    //            //    selectedIndex = components.empty() ? 0 : components.size() - 1;

    //            ImGui::PopID();
    //            break; // Exit loop because vector changed
    //        }
    //        ImGui::PopID();
    //    }

    //    ImGui::PushItemWidth(80);
    //    ImGui::DragFloat("x", &x, 0.1f);
    //    ImGui::PopItemWidth();
    //});

    //fixture->RunGameLoop();

#else
    auto sceneFixture = SceneFixture::GetInstance();
    ASSERT_RESULT(sceneFixture);

    //ASSERT_RESULT(ChainScene::Run(sceneFixture.GetValue()));
    //ASSERT_RESULT(TextScene::Run(sceneFixture.GetValue()));
    //ASSERT_RESULT(SpriteScene::Run(sceneFixture.GetValue()));
    //ASSERT_RESULT(MouseScene::Run(sceneFixture.GetValue()));
    //ASSERT_RESULT(ParticleScene::Run(sceneFixture.GetValue()));
    ASSERT_RESULT(AudioScene::Run(sceneFixture.GetValue()));


#endif
    return 0;
}

 