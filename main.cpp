#include <iostream>
#include <string>
#include "file/FilePathUtility.h"
#include "test/catch/Config.h"

//#define CATCH_CONFIG_RUNNER
#include "deps/catch/catch_amalgamated.hpp"

int main(int argc, char* argv[]) 
{
    FilePathUtility::Init(argv[0]);

#if RUN_CATCH_TESTS

    return Catch::Session().run(argc, argv);

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

 