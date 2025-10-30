#include "../CatchUtils.h"
#include "../test_utils/ImageUtilities.h"
#include "../test_utils/AtlasTestUtils.h"
#include "../../../atlas/TextureRepository.h"
#include "../../../file/FilePathUtility.h"

namespace {

SDL_Color GetRainbowColor(float t)
{
	float r = std::sin(2.0f * M_PI * t + 0.0f) * 0.5f + 0.5f;
	float g = std::sin(2.0f * M_PI * t + 2.0f * M_PI / 3.0f) * 0.5f + 0.5f;
	float b = std::sin(2.0f * M_PI * t + 4.0f * M_PI / 3.0f) * 0.5f + 0.5f;

	return {
		static_cast<uint8_t>(r * 255),
		static_cast<uint8_t>(g * 255),
		static_cast<uint8_t>(b * 255),
		255
	};
}


} // unnamed

TEST_CASE("Sprites rendered correctly", "[rendering]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	auto spriteAtlasResult = SpriteAtlas::Create(SDLite::Renderer(), 2048);
	REQUIRE_RESULT(spriteAtlasResult);

	auto& spriteAtlas = spriteAtlasResult.GetValue();
	CHECK(spriteAtlas.IsLoaded());

	REQUIRE(spriteAtlas.GetSourceTexture() != nullptr);

	auto airbornePathsResult =
		ResourcePaths::SpriteDirectory("knight_new/airborne", std::less<std::string>{});
	REQUIRE_RESULT(airbornePathsResult);

	auto& airbornePaths = airbornePathsResult.GetValue();
	CHECK(airbornePaths.size() == 12);

	auto package = test::MakeSpriteTestPackage(std::move(airbornePaths), "airborne_series");
	CHECK(package.descriptors.size() == 12);
	CHECK(package.seriesName == "airborne_series");

	auto loadResult = spriteAtlas.LoadSprites(SDLite::Renderer(), std::move(package));
	REQUIRE_RESULT(loadResult);

	auto& sprites = loadResult.GetValue();

	SDL_Rect renderRect = { 0, 0, 0, 0 };

	SDL_PumpEvents();
	
	SDLite::Renderer().Clear(SDLite::kColorWhite);
	
	int maxRowHeight = 0;
	for (const auto& sprite : sprites)
	{
		const auto& plotRect = sprite.plot.rect;

		REQUIRE(plotRect.w > 0);
		REQUIRE(plotRect.h > 0);

		maxRowHeight = std::max(maxRowHeight, plotRect.h);

		renderRect.w = plotRect.w;
		renderRect.h = plotRect.h;

		if ((renderRect.x + renderRect.w) > SDLite::kWindowWidth)
		{
			renderRect.x = 0;
			renderRect.y += maxRowHeight;
		}

		SDL_RenderCopy(SDLite::Renderer(), spriteAtlas.GetSourceTexture(), 
					   &plotRect, &renderRect);

		renderRect.x += plotRect.w;
	}

	SDLite::Renderer().Show();

	auto imgWriteResult = test::WriteFrameBufferToPNG(SDLite::Renderer(), 
		"airborne_series_sprites.png");
	REQUIRE_RESULT(imgWriteResult);
	
	SDLite::Exit();
} 


TEST_CASE("Glyphs rendered correctly", "[rendering]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	auto pathResult = ResourcePath::Font("GoNotoKurrent-Regular.ttf");
	REQUIRE_RESULT(pathResult);

	FontDescriptor descriptorReg{
		.fontName = "GoNotoCurrent-Regular",
		.filepath = pathResult.GetValue(),
		.fontSize = 48
	};

	auto loadResult = NewGlyphAtlas::Create(SDLite::Renderer(), std::move(descriptorReg));
	REQUIRE_RESULT(loadResult);

	auto& glyphAtlas = loadResult.GetValue();
	CHECK(glyphAtlas.IsLoaded());
	CHECK(glyphAtlas.GetHandle().IsValid());
	CHECK(glyphAtlas.GetSourceTexture() != nullptr);
	REQUIRE(glyphAtlas.GetFontDescriptor().fontHeight > 0);

	SDL_Rect renderRect = { 0, 0, 0, 0 };

	SDL_PumpEvents();

	SDLite::Renderer().Clear(SDLite::kColorWhite);

	size_t i = 0;
	for (char c = 'a'; c <= 'z'; c++)
	{
		const auto& glyph = glyphAtlas.GetGlyph(c);
		CHECK(glyph.character == c);
		CHECK(glyph.advance > 0);

		const auto& plotRect = glyph.plot.rect;

		REQUIRE(plotRect.w > 0);
		REQUIRE(plotRect.h > 0);

		renderRect.w = plotRect.w;
		renderRect.h = plotRect.h;

		if ((renderRect.x + renderRect.w) > SDLite::kWindowWidth)
		{
			renderRect.x = 0;
			renderRect.y += glyphAtlas.GetFontDescriptor().fontHeight;
		}

		float t = static_cast<float>(i++) / 26;
		SDL_Color clr = GetRainbowColor(t);

		SDL_SetTextureColorMod(glyphAtlas.GetSourceTexture(), clr.r, clr.g, clr.b);

		SDL_RenderCopy(SDLite::Renderer(), glyphAtlas.GetSourceTexture(),
					   &plotRect, &renderRect);

		renderRect.x += plotRect.w;
	}

	SDLite::Renderer().Show();

	auto imgWriteResult = test::WriteFrameBufferToPNG(SDLite::Renderer(),
		"GoNotoCurrent_Regular_font_glyphs_color_mod.png");
	REQUIRE_RESULT(imgWriteResult);

	SDLite::Exit();
}