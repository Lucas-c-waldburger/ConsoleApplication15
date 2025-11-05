#include "../CatchUtils.h"
#include "../../../file/FilePathUtility.h"
#include "../../../sdl/SDLite.h"
#include "../../../atlas/NewTextureRepository.h"
#include "../test_utils/AtlasTestUtils.h"

namespace {

constexpr std::string_view kFontBold = "GoNotoKurrent-Bold.ttf";

} // unnamed


TEST_CASE("Texture Repository tests", "[atlas]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	NewTextureRepository textureRepo{};

	// make a sprite atlas
	auto spriteAtlasResult = SpriteAtlas::Create(SDLite::Renderer());
	REQUIRE_RESULT(spriteAtlasResult);
	CHECK(spriteAtlasResult.GetValue().IsLoaded());

	const auto spriteAtlasHandle = spriteAtlasResult.GetValue().GetHandle();
	CHECK(spriteAtlasHandle.IsValid());

	// attach sprite atlas
	auto spriteAttachResult = textureRepo.AttachAtlas(std::move(spriteAtlasResult.GetValue()));
	CHECK_RESULT(spriteAttachResult);

	// make a glyph atlas 
	auto pathResult = ResourcePath::Font(kFontBold);
	REQUIRE_RESULT(pathResult);

	FontDescriptor fontDesc{
		.fontName = "GoNotoCurrent-Bold",
		.filepath = pathResult.GetValue(),
		.fontSize = 24
	};

	auto glyphAtlasResult = NewGlyphAtlas::Create(SDLite::Renderer(), std::move(fontDesc));
	REQUIRE_RESULT(glyphAtlasResult);
	CHECK(glyphAtlasResult.GetValue().IsLoaded());

	const auto glyphAtlasHandle = glyphAtlasResult.GetValue().GetHandle();
	CHECK(glyphAtlasHandle.IsValid());

	// attach glyph atlas
	auto glyphAttachResult = textureRepo.AttachAtlas(std::move(glyphAtlasResult.GetValue()));
	CHECK_RESULT(glyphAttachResult);

	// retrieve sprite atlas
	auto* retrievedSpriteAtlas = textureRepo.GetAtlas<SpriteAtlas>(spriteAtlasHandle);
	CHECK(retrievedSpriteAtlas != nullptr);

	// retrieve glyph atlas
	auto* retrievedGlyphAtlas = textureRepo.GetAtlas<NewGlyphAtlas>(glyphAtlasHandle);
	CHECK(retrievedGlyphAtlas != nullptr);

	// retrieve sprite atlas src texture
	const auto* retrievedSpriteTexture = textureRepo.GetSourceTexture(spriteAtlasHandle);
	CHECK(retrievedSpriteTexture != nullptr);

	// retrieve glyph atlas src texture
	const auto* retrievedGlyphTexture = textureRepo.GetSourceTexture(glyphAtlasHandle);
	CHECK(retrievedGlyphTexture != nullptr);

	// destroy sprite atlas
	bool spriteAtlasDestroyed = textureRepo.DestroyAtlas(spriteAtlasHandle);
	CHECK(spriteAtlasDestroyed);

	// destroy glyph atlas
	bool glyphAtlasDestroyed = textureRepo.DestroyAtlas(glyphAtlasHandle);
	CHECK(glyphAtlasDestroyed);

	SDLite::Exit();
}

TEST_CASE("Texture Repository can hold multiple atlases of same type", "[atlas]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	NewTextureRepository textureRepo{};

	std::array<Handle<NewTextureAtlas>, 11> spriteAtlasHandles;

	// make and attach sprite atlases
	for (size_t i = 0; i < spriteAtlasHandles.size(); i++)
	{
		auto createResult = SpriteAtlas::Create(SDLite::Renderer());
		REQUIRE_RESULT(createResult);
		CHECK(createResult.GetValue().IsLoaded());

		spriteAtlasHandles[i] = createResult.GetValue().GetHandle();
		CHECK(spriteAtlasHandles[i].IsValid());

		auto attachResult = textureRepo.AttachAtlas(std::move(createResult.GetValue()));
		CHECK_RESULT(attachResult);
	}

	auto airbornePathsResult =
		ResourcePaths::SpriteDirectory("knight_new/airborne", std::less<std::string>{});
	REQUIRE_RESULT(airbornePathsResult);

	auto& airbornePaths = airbornePathsResult.GetValue();
	CHECK(airbornePaths.size() == 12);

	auto airbornePathsCopy = airbornePaths;
	auto package = test::MakeSpriteTestPackage(std::move(airbornePathsCopy), "airborne_series");
	CHECK(package.descriptors.size() == 12);
	CHECK(package.seriesName == "airborne_series");

	// load a sprite on each atlas
	for (size_t i = 0; i < spriteAtlasHandles.size(); i++)
	{
		auto* retrieved = textureRepo.GetAtlas<SpriteAtlas>(spriteAtlasHandles[i]);
		REQUIRE(retrieved != nullptr);
		CHECK(retrieved->GetSourceTexture() != nullptr);

		auto descriptorCopy = package.descriptors[i];
		auto loadResult = retrieved->LoadSprite(SDLite::Renderer(), std::move(descriptorCopy));
		REQUIRE_RESULT(loadResult);
	}

	SDLite::Exit();
}

