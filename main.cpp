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

    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    io.FontGlobalScale = 1.5f;
    style.ScaleAllSizes(1.3f);

    static constexpr std::array kComponentNames = {
        "Transform", "SpriteRenderable", "TextRenderable"
    };
    size_t selectedIndex = 0;
    float x = 0.0f;

    gui->AddWidget("Components", [&] {
        for (size_t i = 0; i < kComponentNames.size(); i++)
        {
            const bool isSelected = (selectedIndex == i);

            if (ImGui::Selectable(kComponentNames[i], isSelected))
            {
                selectedIndex = i; // clicked
            }

            ImGui::SameLine(ImGui::GetWindowWidth() - 30);  // Push to right side

            ImGui::PushID((int)i);  // To avoid ID collisions
            if (ImGui::SmallButton("X"))
            {
                // Remove component
                //components.erase(components.begin() + i);

                // Fix selection
                //if (selectedIndex >= components.size())
                //    selectedIndex = components.empty() ? 0 : components.size() - 1;

                ImGui::PopID();
                break; // Exit loop because vector changed
            }
            ImGui::PopID();
        }

        ImGui::PushItemWidth(80);
        ImGui::DragFloat("x", &x, 0.1f);
        ImGui::PopItemWidth();
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

 