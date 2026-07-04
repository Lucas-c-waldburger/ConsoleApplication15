#include "../CatchUtils.h"
#include "../../../file/FilePathUtility.h"
#include "../../../sdl/SDLite.h"
#include "../../../atlas/NewTextureRepository.h"
#include "../test_utils/AtlasTestUtils.h"

namespace {

constexpr std::string_view kFontReg = "GoNotoKurrent-Regular.ttf";
constexpr std::string_view kFontBold = "GoNotoKurrent-Bold.ttf";
constexpr std::string_view kSpriteFall = "knight/fall_anim/knight_fall_0.png";

} // unnamed


TEST_CASE("Texture Repository tests", "[atlas]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	TextureRepository textureRepo{};
	auto& spriteAtlas = textureRepo.GetSpriteAtlas();
	auto& fontAtlas = textureRepo.GetFontAtlas();

	// load a sprite
	auto spritePathResult = ResourcePath::Sprite(kSpriteFall);
	REQUIRE_RESULT(spritePathResult);

	auto spriteLoadResult = spriteAtlas.LoadSprite(SDLite::Renderer(),
		{ .filepath = spritePathResult.GetValue() });
	REQUIRE_RESULT(spriteLoadResult);

	const auto& sprite = spriteLoadResult.GetValue();
	REQUIRE(spriteAtlas.IsSpriteValid(sprite));

	// load a font
	auto fontPathResult = ResourcePath::Font(kFontBold);
	REQUIRE_RESULT(fontPathResult);

	auto fontLoadResult = fontAtlas.LoadFont(SDLite::Renderer(), {
		.fontName = "GoNotoCurrent-Bold",
		.filepath = fontPathResult.GetValue(),
		.fontSize = 24
	});
	REQUIRE_RESULT(fontLoadResult);
	REQUIRE(textureRepo.GetFontAtlas().HasFont("GoNotoCurrent-Bold"));

	const auto& font = textureRepo.GetFontAtlas().GetFont("GoNotoCurrent-Bold");
	REQUIRE(font.IsLoaded());

	// retrieve sprite atlas texture info
	auto spriteAtlasIdOp = spriteAtlas.GetSpriteInfo<&SpriteInfo::atlasId>(sprite);
	REQUIRE(spriteAtlasIdOp.has_value());

	const auto& spriteAtlasId = *spriteAtlasIdOp;
	CHECK(spriteAtlasId == sprite.resourceHandle.GetAtlasID());

	// retrieve font text writer info
	const auto writer = fontAtlas.GetTextWriter("GoNotoCurrent-Bold");
	CHECK(writer.resourceHandle.IsValid());
	CHECK(writer.resourceHandle.GetAtlasID() == font.GetAtlasID());

	// retrieve sprite atlas src texture
	const auto* retrievedSpriteTexture = 
		textureRepo.GetSourceTexture(sprite.resourceHandle);
	CHECK(retrievedSpriteTexture != nullptr);

	// retrieve glyph atlas src texture
	const auto* retrievedFontTexture = textureRepo.GetSourceTexture(writer.resourceHandle);
	CHECK(retrievedFontTexture != nullptr);

	SDLite::Exit();
}

TEST_CASE("TextureRepository reacts to texture creation", "[atlas]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	TextureRepository repo{};
	auto& spriteAtlas = repo.GetSpriteAtlas();
	auto& fontAtlas = repo.GetFontAtlas();

	CHECK(repo.GetSpriteAtlas().GetTextureCount() == 0);
	CHECK(repo.GetFontAtlas().GetTextureCount() == 0);

	// load a sprite
	auto spritePathResult = ResourcePath::Sprite(kSpriteFall);
	REQUIRE_RESULT(spritePathResult);

	auto spriteLoadResult = spriteAtlas.LoadSprite(SDLite::Renderer(),
		{ .filepath = spritePathResult.GetValue() });
	REQUIRE_RESULT(spriteLoadResult);

	const auto& sprite = spriteLoadResult.GetValue();
	REQUIRE(spriteAtlas.IsSpriteValid(sprite));

	// check that the sprite texture was updated & stored in repo
	CHECK(repo.GetSpriteAtlas().GetTextureCount() == 1);

	const auto* spriteSrcTexture = repo.GetSourceTexture(sprite.resourceHandle);
	CHECK(spriteSrcTexture != nullptr);

	// load a font
	auto fontPathResult = ResourcePath::Font(kFontBold);
	REQUIRE_RESULT(fontPathResult);

	auto fontLoadResult = fontAtlas.LoadFont(SDLite::Renderer(), {
		.fontName = "GoNotoCurrent-Bold",
		.filepath = fontPathResult.GetValue(),
		.fontSize = 24
		});
	REQUIRE_RESULT(fontLoadResult);
	REQUIRE(fontAtlas.HasFont("GoNotoCurrent-Bold"));

	const auto& font = fontAtlas.GetFont("GoNotoCurrent-Bold");
	REQUIRE(font.IsLoaded());

	// get writer to get resource handle
	auto writer = fontAtlas.GetTextWriter("GoNotoCurrent-Bold");
	CHECK(writer.resourceHandle.IsValid());
	CHECK(writer.resourceHandle.GetAtlasID() == font.GetAtlasID());

	// check that the font texture was updated & stored in repo
	CHECK(repo.GetFontAtlas().GetTextureCount() == 1);

	const auto* fontSrcTexture = repo.GetSourceTexture(writer.resourceHandle);
	CHECK(fontSrcTexture != nullptr);

	SDLite::Exit();
}