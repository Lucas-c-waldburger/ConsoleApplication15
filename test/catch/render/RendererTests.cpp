#include "../CatchUtils.h"
#include "../test_utils/ImageUtilities.h"
#include "../test_utils/AtlasTestUtils.h"
#include "../../../atlas/NewTextureRepository.h"
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

	TextureRepository textureRepo{};
	auto& spriteAtlas = textureRepo.GetSpriteAtlas();

	auto airbornePathsResult =
		ResourcePaths::SpriteDirectory("knight_new/airborne", std::less<std::string>{});
	REQUIRE_RESULT(airbornePathsResult);

	auto& airbornePaths = airbornePathsResult.GetValue();
	CHECK(airbornePaths.size() == 12);

	auto package = test::MakeSpriteTestPackage(std::move(airbornePaths), "airborne_series");
	CHECK(package.data.size() == 12);
	CHECK(package.seriesName == "airborne_series");

	auto loadResult = spriteAtlas.LoadSprites(SDLite::Renderer(), std::move(package));
	REQUIRE_RESULT(loadResult);

	auto& sprites = loadResult.GetValue();
	REQUIRE_FALSE(sprites.empty());

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

		if ((renderRect.x + renderRect.w) > SDLite::Window().GetSize().w)
		{
			renderRect.x = 0;
			renderRect.y += maxRowHeight;
		}

		auto* srcTexture = textureRepo.GetSourceTexture(sprite.resourceHandle);
		REQUIRE(srcTexture);

		SDL_RenderCopy(SDLite::Renderer(), srcTexture, 
					   &plotRect, &renderRect);

		renderRect.x += plotRect.w;
	}

	SDLite::Renderer().Show();

	auto imgWriteResult = test::WriteFrameBufferToPNG(SDLite::Renderer(), 
		"airborne_series_sprites_NEW.png");
	REQUIRE_RESULT(imgWriteResult);
	
	SDLite::Exit();
} 


TEST_CASE("Glyphs rendered correctly", "[rendering]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	TextureRepository textureRepo{};
	auto& fontAtlas = textureRepo.GetFontAtlas();

	auto pathResult = ResourcePath::Font("GoNotoKurrent-Regular.ttf");
	REQUIRE_RESULT(pathResult);

	FontDescriptor descriptorReg{
		.fontName = "GoNotoCurrent-Regular",
		.filepath = pathResult.GetValue(),
		.fontSize = 48
	};

	auto loadResult = fontAtlas.LoadFont(SDLite::Renderer(), std::move(descriptorReg));
	REQUIRE_RESULT(loadResult);

	const auto& font = fontAtlas.GetFont("GoNotoCurrent-Regular");
	CHECK(font.IsLoaded());
	CHECK(font.GetAtlasID() != kInvalidTextureAtlasID);
	CHECK(font.GetSourceTexture() != nullptr);

	const auto fontHeightOp = fontAtlas.GetFontInfo<&FontInfo::fontHeight>(
		"GoNotoCurrent - Regular"
	);
	REQUIRE(fontHeightOp.has_value());
	const auto [fontHeight] = *fontHeightOp;
	REQUIRE(fontHeight > 0);

	SDL_Rect renderRect = { 0, 0, 0, 0 };

	SDL_PumpEvents();

	SDLite::Renderer().Clear(SDLite::kColorWhite);

	size_t i = 0;
	for (char c = 'a'; c <= 'z'; c++)
	{
		const auto& glyph = font.GetGlyph(c);
		CHECK(glyph.character == c);
		CHECK(glyph.advance > 0);

		const auto& plotRect = glyph.plot.rect;

		REQUIRE(plotRect.w > 0);
		REQUIRE(plotRect.h > 0);

		renderRect.w = plotRect.w;
		renderRect.h = plotRect.h;

		if ((renderRect.x + renderRect.w) > SDLite::Window().GetSize().w)
		{
			renderRect.x = 0;
			renderRect.y += fontHeight;
		}

		float t = static_cast<float>(i++) / 26;
		SDL_Color clr = GetRainbowColor(t);

		SDL_SetTextureColorMod(font.GetSourceTexture(), clr.r, clr.g, clr.b);

		SDL_RenderCopy(SDLite::Renderer(), font.GetSourceTexture(),
					   &plotRect, &renderRect);

		renderRect.x += plotRect.w;
	}

	SDLite::Renderer().Show();

	auto imgWriteResult = test::WriteFrameBufferToPNG(SDLite::Renderer(),
		"GoNotoCurrent_Regular_font_glyphs_color_mod.png");
	REQUIRE_RESULT(imgWriteResult);

	SDLite::Exit();
}