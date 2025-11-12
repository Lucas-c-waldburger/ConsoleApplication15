#include <iostream>
#include <string>
#include "file/FilePathUtility.h"
#include "test/catch/Config.h"
#include "deps/catch/catch_amalgamated.hpp"
#include "test/Fixtures.h"
#include "test/ui/Workspace.h"

int main(int argc, char* argv[]) 
{
    FilePathUtility::Init(argv[0]);

#if RUN_MODE(RUN_CATCH_TESTS)

    return Catch::Session().run(argc, argv);

#elif RUN_MODE(RUN_UI_VISUALIZER)

    auto fixResult = SceneFixture::GetInstance();
    ASSERT_RESULT(fixResult);
    auto& fixture = fixResult.GetValue();

    auto& repo = fixture->GetTextureRepository();

    auto atlasResult = fixture->LoadGlyphAtlas(
        ResourcePath::Font("GoNotoKurrent-Bold.ttf"), 24);
    ASSERT_RESULT(atlasResult);
    auto& glyphAtlas = atlasResult.GetValue();

    auto entity = ECS::CreateEntity();

    auto& tf = entity.AddComponent(Transform{
        .position = { SDLite::kFWindowCenter.x - 100.0f,
                      SDLite::kFWindowCenter.y - 100.0f } 
    });

    auto& textRenderable = entity.AddComponent(TextRenderableComponent{
        .writer = glyphAtlas->GetTextWriter(),
        .formatting = { .bounds = { 100, 500 } }
    });

    static constexpr std::string_view kYouPressedIt = "Yay you pressed it!";

    auto wsResult = ui::Workspace::Create(
        fixture->GetRenderer(),
        fixture->GetTextureRepository(),
        fixture->GetEventBus()
    );
    ASSERT_RESULT(wsResult);

    auto& workspace = wsResult.GetValue();

    auto handleResult = workspace.PlaceButton({
        .position = SDLite::kFWindowCenter,
        .color = ui::Button::Color::Green,
        .onClick = [entity]() mutable {
            entity.GetComponent<TextRenderableComponent>().writer.text = kYouPressedIt;
        }
    });

    ASSERT_RESULT(fixture->RunGameLoop());

#else
    auto sceneFixture = SceneFixture::GetInstance();
    ASSERT_RESULT(sceneFixture);

    //ASSERT_RESULT(ChainScene::Run(sceneFixture.GetValue()));
    //ASSERT_RESULT(TextScene::Run(sceneFixture.GetValue()));
    //ASSERT_RESULT(SpriteScene::Run(sceneFixture.GetValue()));
    //ASSERT_RESULT(MouseScene::Run(sceneFixture.GetValue()));
    //ASSERT_RESULT(ParticleScene::Run(sceneFixture.GetValue()));
    ASSERT_RESULT(AudioScene::Run(sceneFixture.GetValue()));

    return 0;

#endif
}

 