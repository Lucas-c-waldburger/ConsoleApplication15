#include <iostream>
#include <coroutine>
#include <variant>
#include <exception>
#include <string>
#include "sdl/SDLite.h"
#include "systems/RenderSystem.h"
#include "scripting/ScriptManager.h"
#include "core/Monitoring.h"
#include "events/EventSystem.h"
#include "ecs/ECS.h"
#include "atlas/AtlasManager.h"

//// TODO
// rethink handle manager being yet another tuple map
// maybe make componentBit non-constexpr implemenation again

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

int main(int argc, char* argv[]) 
{
    Logger::StartSession();
    SDLite::Start();
      
    Entity entA = ECS::CreateEntity();

    impl::AtlasStore atlasStore{};
    auto spriteHandleResult = atlasStore.LoadAtlas(SDLite::Renderer(), MakeKnightAtlasInfo());
    assert(spriteHandleResult.Success());

    auto spriteAtlas = atlasStore.GetAtlas(*spriteHandleResult);
    assert(spriteAtlas);

    SDL_Rect spriteRect = spriteAtlas->GetSprite(kWalkSeriesName, 0).atlasRect;

    entA.AddComponent(Renderable{
        .renderData = Renderable::Sprite{
            .sourceAtlas = *spriteHandleResult,
            .seriesName = std::string{kWalkSeriesName},
            .currentIndex = 0
        },
        .drawOrder = 0  
        }
    );
    auto& spatialA = entA.AddComponent(Spatial{
        .position = SDL_FPoint{ SDLite::kWindowWidth / 2.0f, SDLite::kWindowHeight / 2.0f },
        .dimensions = {static_cast<float>(spriteRect.w), static_cast<float>(spriteRect.h)}
        }
    );
    auto& tfA = entA.AddComponent(Transform{});
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

    RenderSystem renderSys{};
    EventSystem eventSystem{};

    SDL_Event ev;
    while (true)
    {
        if (!eventSystem.Poll(ev))
        {
            break;
        }

        eventSystem.DistributeEvents();

        //fileMonitor.Run(GetDeltaTime());
        //if (fileMonitor.Check())
        //{
        //    lua.Run();
        //}

        SDLite::Renderer().Clear();

        //SDL_Rect tfRect = MakeTransformedRect(bSpatial, bTf);

        //SDL_SetRenderDrawColor(SDLite::Renderer(), 255, 0, 0, 255);
        //SDL_RenderDrawRect(SDLite::Renderer(), &tfRect);
        //SDL_SetRenderDrawColor(SDLite::Renderer(), 0xFF, 0xFF, 0xFF, 0xFF);

        renderSys.Update(SDLite::Renderer(), atlasStore);

        //SDLite::Canvas().Draw(SDLite::Renderer());

        //int xPos = 0;
        //for (const auto& glyph : glyphs)
        //{
        //    SDL_Rect dest = { xPos, 0, glyph.atlasRect.w, glyph.atlasRect.h };

        //    SDL_RenderCopy(SDLite::Renderer(), glyphAtlas.GetAtlasTexture(), &glyph.atlasRect, &dest);

        //    xPos += glyph.advance;
        //}

        //SDL_Rect destRect = firstSprite.atlasRect;
        //destRect.x = 0;
        //destRect.y = 0;

        //SDL_RenderCopy(SDLite::Renderer(), spriteSeriesAtlas.GetAtlasTexture(), &firstSprite.atlasRect, &destRect);

        SDLite::Renderer().Show();
    }

    Logger::EndSession();

    SDLite::Exit();

    return 0;
}

 