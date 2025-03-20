#include <iostream>
#include <coroutine>
#include <variant>
#include <exception>
#include <string>
#include "SDLite.h"
#include "Systems.h"

static constexpr std::string_view kSpritesPath = R"(C:\Users\Lucas\source\repos\ConsoleApplication15\sprites)";
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
    SDLite::Start();

    //static constexpr const char* kFontPath = 
    //    R"(C:\Windows\WinSxS\amd64_microsoft-windows-font-truetype-arial_31bf3856ad364e35_10.0.19041.1_none_28747db34cb89a67\arial.ttf)";

    //GlyphAtlas glyphAtlas{};

    //glyphAtlas.Load(SDLite::Renderer(), { .fontPath = kFontPath, .fontSize = 48, .fontColor = {51, 255, 196, 255} });

    //static constexpr std::string_view kTestText = "Bubba is fat and it's becoming a problem";
    //auto glyphs = glyphAtlas.GetGlyphsForString(kTestText);

    //SpriteSeriesAtlas spriteSeriesAtlas{};

    //spriteSeriesAtlas.Load(SDLite::Renderer(), MakeKnightAtlasInfo());

    //auto firstSprite = spriteSeriesAtlas.GetSprite(kWalkSeriesName, 0);

    //SDLite::Scripts().AddScript(R"(
    //    function add(a, b)
    //        return a + b
    //    end
    //)");


    //sol::state lua;
    //lua.open_libraries(sol::lib::base);

    //// Run a Lua script that returns a value
    //lua.script(R"(
    //    function add(a, b)
    //        return a + b
    //    end
    //)");

    //// Call the Lua function and get the result
    //sol::function add = lua["add"];
    //int result = add(3, 4);  // Calling Lua function 'add' with arguments 3 and 4

    //std::cout << "Result from Lua: " << result << std::endl;
    //SDLite::Rectangle rect{};
    //rect.x = 200;
    //rect.y = 200;
    //rect.w = 100; 
    //rect.h = 100;
    //rect.color = { .values = { 0, 0, 0, 255 }, .option = SDLite::Color::Fill };

    //auto rectPtr = SDLite::Canvas().AddObject(std::move(rect));

    Entity entA = ECS::CreateEntity();

    RenderSystem renderSys{};
    auto handleResult = renderSys.LoadAtlas(SDLite::Renderer(), MakeKnightAtlasInfo());
    assert(handleResult.Success());

    entA.AddComponent(Renderable{
        .renderData = Renderable::Sprite{
            .sourceAtlas = *handleResult,
            .seriesName = std::string{kWalkSeriesName},
            .currentIndex = 0
        },
        .drawOrder = 0 
        }
    );

    entA.AddComponent(Spatial{
        .position = SDL_FPoint{ SDLite::kWindowWidth / 2.0f, SDLite::kWindowHeight / 2.0f },
        .dimensions = Dimensions<float>{200, 200}
        }
    );

    entA.AddComponent(Transform{});

    while (true)
    {
        if (!SDLite::Events().Process())
        {
            break;
        }

        SDLite::Renderer().Clear();

        renderSys.Update(SDLite::Renderer());


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

    SDLite::Exit();

    return 0;
}

 