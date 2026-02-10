#include "../CatchUtils.h"
#include "../../../atlas/SpriteAtlasCollection.h"
#include "../../../file/FilePathUtility.h"
#include "../../../sdl/SDLite.h"
#include "../test_utils/AtlasTestUtils.h"

namespace {

struct MockTextureObserver : public TextureObserver
{
	std::unordered_map<TextureAtlasID, SDL_Texture*> atlasIdToTexture;
	SignalToken token;

	void Connect(SpriteAtlas& collection)
	{
		token = collection.ConnectTextureObserver(GetTextureObserverPassKey(),
			[this](TextureAtlasID id, SDL_Texture* tx) {
				atlasIdToTexture[id] = tx;
			});
	}
};

} // unnamed

TEST_CASE("SpriteAtlas Tests", "[atlas]")
{
	auto pathResult = ResourcePath::Sprite("knight/jump_anim/knight_jump_0.png");
	REQUIRE(pathResult.Success());

	const auto& path = pathResult.GetValue();

	SpriteDescriptor knightJumpSpriteDescriptor{
		.spriteName = "knight_jump_0",
		.filepath = path
	};

	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	SpriteAtlas spriteAtlas{};

	{
		// Load single sprite
		auto loadResult = spriteAtlas.LoadSprite(
			SDLite::Renderer(), std::move(knightJumpSpriteDescriptor)
		);
		REQUIRE_RESULT(loadResult);

		auto& sprite = loadResult.GetValue();
		CHECK(sprite.resourceHandle.IsValid());
		CHECK(sprite.plot.rect.w >= 128);
		CHECK(sprite.plot.rect.h >= 128);

		CHECK(spriteAtlas.IsSpriteValid(sprite));

		// get info
		const auto info = spriteAtlas.GetSpriteInfo(sprite);
		REQUIRE(info.has_value());

		const auto& [atlasId, plot, name, filepath, series, seriesIdx] = *info;
		CHECK(atlasId == sprite.resourceHandle.GetAtlasID());
		CHECK(name == "knight_jump_0");
		CHECK(filepath == path);
		CHECK(series.empty());
		CHECK(seriesIdx == kSizeMax);

		// retrieve it
		auto retrievedSprite = spriteAtlas.GetSprite("knight_jump_0");
		CHECK(retrievedSprite == sprite);
	}

	// load series on same atlas
	{
		auto seriesPathsResult =
			ResourcePaths::SpriteDirectory("knight/fall_anim", std::less<std::string>{});
		REQUIRE_RESULT(seriesPathsResult);

		auto& seriesPaths = seriesPathsResult.GetValue();
		CHECK(seriesPaths.size() == 4);

		auto package = test::MakeSpriteTestPackage(std::move(seriesPaths), 
			"knight_fall_series");
		CHECK(package.data.size() == 4);
		CHECK(package.seriesName == "knight_fall_series");

		auto loadResult = spriteAtlas.LoadSprites(SDLite::Renderer(),
			std::move(package));
		REQUIRE_RESULT(loadResult);

		auto& sprites = loadResult.GetValue();
		CHECK(sprites.size() == 4);

		for (size_t i = 0; i < sprites.size(); i++)
		{
			CHECK(sprites[i].resourceHandle.IsValid());
			CHECK(sprites[i].plot.rect.w >= 128);
			CHECK(sprites[i].plot.rect.h >= 128);
			CHECK(static_cast<size_t>(sprites[i].resourceHandle.GetResourceIndex())
				  == i + 1); // <- since we already loaded 1 previously
		}

		// validate them
		for (const auto& sp : sprites)
		{
			CHECK(spriteAtlas.IsSpriteValid(sp));
		}

		// get info	
		for (size_t i = 0; i < sprites.size(); i++)
		{
			auto info = spriteAtlas.GetSpriteInfo(sprites[i]);
			REQUIRE(info.has_value());
			const auto& [atlasId, plot, name, path, series, seriesIdx] = *info;
			CHECK(atlasId == sprites[i].resourceHandle.GetAtlasID());
			CHECK(name == std::format("knight_fall_{}", i));
			CHECK(path == seriesPaths[i]);
			CHECK(series == "knight_fall_series");
			CHECK(seriesIdx == i);
		}

		// retrieve them
		auto retrievedSpriteSeries = spriteAtlas.GetSpriteSeries(
			"knight_fall_series");
		CHECK_FALSE(retrievedSpriteSeries.empty());
		CHECK(sprites == retrievedSpriteSeries);
	}

	SDLite::Exit();
}

TEST_CASE("Loading SpriteAtlas until exceeds original texture size", "[atlas]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	SpriteAtlas spriteAtlasCollection{128};

	auto seriesPathsResult =
		ResourcePaths::SpriteDirectory("knight/fall_anim", std::less<std::string>{});
	REQUIRE_RESULT(seriesPathsResult);

	auto& seriesPaths = seriesPathsResult.GetValue();
	CHECK(seriesPaths.size() == 4);

	auto package = test::MakeSpriteTestPackage(std::move(seriesPaths),
		"knight_fall_series");
	CHECK(package.data.size() == 4);
	CHECK(package.seriesName == "knight_fall_series");

	auto loadResult = spriteAtlasCollection.LoadSprites(SDLite::Renderer(),
		std::move(package));
	REQUIRE_RESULT(loadResult);

	CHECK(spriteAtlasCollection.GetTextureCount() == 4);

	const auto& sprites = loadResult.GetValue();
	for (const auto& sp : sprites)
	{
		CHECK(spriteAtlasCollection.IsSpriteValid(sp));
	}
}

TEST_CASE("SpriteAtlas Texture Created Notification", "[atlas]")
{
	SpriteAtlas spriteAtlas{};
	CHECK(spriteAtlas.GetTextureCount() == 0);

	MockTextureObserver mockObserver{};
	mockObserver.Connect(spriteAtlas);

	CHECK(mockObserver.token.IsConnected());
	CHECK(mockObserver.atlasIdToTexture.empty());

	auto pathResult = ResourcePath::Sprite("knight/jump_anim/knight_jump_0.png");
	REQUIRE(pathResult.Success());

	const auto& path = pathResult.GetValue();

	SpriteDescriptor knightJumpSpriteDescriptor{
		.spriteName = "knight_jump_0",
		.filepath = path
	};

	auto loadResult = spriteAtlas.LoadSprite(
		SDLite::Renderer(), std::move(knightJumpSpriteDescriptor)
	);
	REQUIRE_RESULT(loadResult);

	auto& sprite = loadResult.GetValue();
	CHECK(sprite.resourceHandle.IsValid());
	CHECK(sprite.plot.rect.w >= 128);
	CHECK(sprite.plot.rect.h >= 128);
	CHECK(sprite.resourceHandle.GetResourceIndex() == 0);

	CHECK(spriteAtlas.GetTextureCount() == 1);
	REQUIRE(spriteAtlas.HasSprite("knight_jump_0"));
	CHECK(spriteAtlas.IsSpriteValid(sprite));
	 
	CHECK(mockObserver.token.IsConnected());
	REQUIRE(mockObserver.atlasIdToTexture.size() == 1);

	const auto& [atlasId, texture] = *mockObserver.atlasIdToTexture.begin();
	CHECK(atlasId != kInvalidTextureAtlasID);
	CHECK(atlasId == sprite.resourceHandle.GetAtlasID());
	CHECK(texture != nullptr);
}