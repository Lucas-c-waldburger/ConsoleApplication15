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
    auto fixtureResult = SceneFixture::GetInstance();
    ASSERT_RESULT(fixtureResult);
    
    auto& fixture = fixtureResult.GetValue();

    auto& gui = fixture->GetSystem<GuiSystem>();

    gui->AddWidget("my window", [] {
        if (ImGui::Button("Press me!"))
        {
            ImGui::Text("You pressed it!");
        }
    });

    fixture->RunGameLoop();

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

 