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
		const auto info = spriteAtlas.GetSpriteInfo<&SpriteInfo::atlasId,
												    &SpriteInfo::plot,
													&SpriteInfo::spriteName,
													&SpriteInfo::filepath>(sprite);
		REQUIRE(info.has_value());

		const auto& [atlasId, plot, name, filepath] = *info;
		CHECK(atlasId == sprite.resourceHandle.GetAtlasID());
		CHECK(name == "knight_jump_0");
		CHECK(filepath == path);

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
			const auto info = spriteAtlas.GetSpriteInfo<&SpriteInfo::atlasId,
													    &SpriteInfo::plot,
													    &SpriteInfo::spriteName,
													    &SpriteInfo::filepath>(sprites[i]);
			REQUIRE(info.has_value());
			const auto& [atlasId, plot, name, path] = *info;
			CHECK(atlasId == sprites[i].resourceHandle.GetAtlasID());
			CHECK(name == std::format("knight_fall_{}", i));
			CHECK(path == seriesPaths[i]);

			// check series
			CHECK(spriteAtlas.HasSpriteSeries("knight_fall_series"));
			CHECK(spriteAtlas.GetSpriteSeriesMember("knight_fall_series", i) == sprites[i]);
			CHECK(spriteAtlas.GetSpriteSeriesMemberIndex("knight_fall_series", sprites[i]) == i);
		}

		// retrieve them
		auto retrievedSpriteSeries = spriteAtlas.GetSpriteSeries(
			"knight_fall_series");
		CHECK_FALSE(retrievedSpriteSeries.empty());
		CHECK(sprites == retrievedSpriteSeries);
	}

	SDLite::Exit();
	Logger::EndSession();
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

	SDLite::Exit();
	Logger::EndSession();
}

TEST_CASE("SpriteAtlas Texture Created Notification", "[atlas]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

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

	SDLite::Exit();
	Logger::EndSession();
}

TEST_CASE("Sprite Series API", "[atlas]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	SpriteAtlas spriteAtlas{};
	auto seriesPathsResult =
		ResourcePaths::SpriteDirectory("knight/fall_anim", std::less<std::string>{});
	REQUIRE_RESULT(seriesPathsResult);

	auto& seriesPaths = seriesPathsResult.GetValue();
	CHECK(seriesPaths.size() == 4);

	SECTION("Sprite series defined automatically by descriptors")
	{
		auto descriptors = test::MakeSpriteTestPackage(std::move(seriesPaths),
			"knight_fall_series");
		CHECK(descriptors.data.size() == 4);
		CHECK(descriptors.seriesName == "knight_fall_series");

		auto loadResult = spriteAtlas.LoadSprites(SDLite::Renderer(),
			std::move(descriptors));
		REQUIRE_RESULT(loadResult);
		CHECK(spriteAtlas.HasSpriteSeries("knight_fall_series"));
		CHECK(spriteAtlas.GetSpriteSeriesSize("knight_fall_series") == 4);

		const auto& sprites = loadResult.GetValue();
		for (size_t i = 0; i < sprites.size(); ++i)
		{
			const auto& sp = sprites[i];

			CHECK(spriteAtlas.IsSpriteValid(sp));
			CHECK(spriteAtlas.GetSpriteSeriesMember("knight_fall_series", i) == sp);
			CHECK(spriteAtlas.GetSpriteSeriesMemberIndex("knight_fall_series", sp) == i);
		}
	}

	SECTION("Sprite series defined manually")
	{
		auto descriptors = test::MakeSpriteTestPackage(std::move(seriesPaths));
		CHECK(descriptors.data.size() == 4);
		CHECK(descriptors.seriesName.empty());

		auto loadResult = spriteAtlas.LoadSprites(SDLite::Renderer(),
			std::move(descriptors));
		REQUIRE_RESULT(loadResult);
		CHECK_FALSE(spriteAtlas.HasSpriteSeries("knight_fall_series"));
		CHECK(spriteAtlas.GetSpriteSeriesSize("knight_fall_series") == 0);

		auto& sprites = loadResult.GetValue();

		// define series
		auto defineResult = spriteAtlas.DefineSpriteSeries("knight_fall_series", sprites);
		REQUIRE_RESULT(defineResult);

		CHECK(spriteAtlas.HasSpriteSeries("knight_fall_series"));
		CHECK(spriteAtlas.GetSpriteSeriesSize("knight_fall_series") == sprites.size());

		for (size_t i = 0; i < sprites.size(); i++)
		{
			CHECK(spriteAtlas.GetSpriteSeriesMember("knight_fall_series", i) == sprites[i]);
			CHECK(spriteAtlas.GetSpriteSeriesMemberIndex("knight_fall_series", sprites[i]) == i);
		}
	}

	SECTION("Removing sprite series members and definition")
	{ 
		auto descriptors = test::MakeSpriteTestPackage(std::move(seriesPaths),
			"knight_fall_series");
		CHECK(descriptors.data.size() == 4);
		CHECK(descriptors.seriesName == "knight_fall_series");

		auto loadResult = spriteAtlas.LoadSprites(SDLite::Renderer(),
			std::move(descriptors));
		REQUIRE_RESULT(loadResult);
		CHECK(spriteAtlas.HasSpriteSeries("knight_fall_series"));
		CHECK(spriteAtlas.GetSpriteSeriesSize("knight_fall_series") == 4);

		const auto& sprites = loadResult.GetValue();
		// remove series member
		REQUIRE(sprites.size() == 4);
		const auto& memberToRemove = sprites[2];

		const bool removedMember = spriteAtlas.RemoveSpriteSeriesMember(
			"knight_fall_series", memberToRemove);
		CHECK(removedMember);

		CHECK(spriteAtlas.GetSpriteSeriesSize("knight_fall_series") == 3);
		CHECK(spriteAtlas.GetSpriteSeriesMemberIndex("knight_fall_series", memberToRemove) ==
			std::numeric_limits<size_t>::max());

		// erase whole series
		const bool removedSeries = spriteAtlas.RemoveSpriteSeries("knight_fall_series");
		CHECK(removedSeries);

		CHECK_FALSE(spriteAtlas.HasSpriteSeries("knight_fall_series"));
		CHECK(spriteAtlas.GetSpriteSeriesSize("knight_fall_series") == 0);

		for (size_t i = 0; i < sprites.size(); i++)
		{
			CHECK(spriteAtlas.GetSpriteSeriesMember("knight_fall_series", i) == Sprite{});
			CHECK(spriteAtlas.GetSpriteSeriesMemberIndex("knight_fall_series", sprites[i]) ==
				std::numeric_limits<size_t>::max());
		}
	}

	SDLite::Exit();
	Logger::EndSession();
}

TEST_CASE("SpriteAtlas::EraseSprite", "[atlas][l]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	SpriteAtlas spriteAtlas{};

	auto seriesPathsResult =
		ResourcePaths::SpriteDirectory("knight/fall_anim", std::less<std::string>{});
	REQUIRE_RESULT(seriesPathsResult);

	auto& seriesPaths = seriesPathsResult.GetValue();
	CHECK(seriesPaths.size() == 4);
	auto descriptors = test::MakeSpriteTestPackage(std::move(seriesPaths),
		"knight_fall_series");
	CHECK(descriptors.data.size() == 4);
	CHECK(descriptors.seriesName == "knight_fall_series");

	auto tempDescriptors = descriptors; // copy for later use
	auto loadResult = spriteAtlas.LoadSprites(SDLite::Renderer(),
		std::move(tempDescriptors));
	REQUIRE_RESULT(loadResult);

	const auto& sprites = loadResult.GetValue();
	for (const auto& sp : sprites)
	{
		CHECK(spriteAtlas.IsSpriteValid(sp));
		CHECK(spriteAtlas.GetSpriteInfo(sp).has_value());
	}

	// erase a sprite
	const auto& spriteToErase = sprites[0];
	CHECK(spriteAtlas.IsSpriteValid(spriteToErase));

	const bool erased = spriteAtlas.EraseSprite(spriteToErase);
	CHECK(erased);
	CHECK_FALSE(spriteAtlas.IsSpriteValid(spriteToErase));

	CHECK_FALSE(spriteAtlas.GetSpriteInfo(spriteToErase));

	// replace it
	const auto& spriteToLoadOvertop = sprites[1];
	CHECK(spriteAtlas.IsSpriteValid(spriteToLoadOvertop));

	REQUIRE(spriteToLoadOvertop.plot.rect.w <= spriteToErase.plot.rect.w);
	REQUIRE(spriteToLoadOvertop.plot.rect.h <= spriteToErase.plot.rect.h);

	auto descriptorToLoadOvertop = descriptors.data[1];
	auto descriptorToLoadOvertopStoredFilepath =
		spriteAtlas.GetSpriteInfo<&SpriteInfo::filepath>(sprites[1]);
	REQUIRE(descriptorToLoadOvertopStoredFilepath.has_value());
	CHECK(*descriptorToLoadOvertopStoredFilepath == descriptorToLoadOvertop.filepath);

	descriptorToLoadOvertop.spriteName = "fake name"; // if same name, will just return orig sprite
	auto loadOvertopResult = spriteAtlas.LoadSprite(SDLite::Renderer(),
		std::move(descriptorToLoadOvertop));
	REQUIRE(loadOvertopResult.Success());
		
	auto& spriteLoadedOvertop = loadOvertopResult.GetValue();
	CHECK(spriteAtlas.IsSpriteValid(spriteLoadedOvertop));

	CHECK(spriteLoadedOvertop.plot.rect.x == spriteToErase.plot.rect.x);
	CHECK(spriteLoadedOvertop.plot.rect.y == spriteToErase.plot.rect.y);

	CHECK(spriteLoadedOvertop.resourceHandle.GetAtlasID() == 
		spriteToErase.resourceHandle.GetAtlasID());
	CHECK(spriteLoadedOvertop.resourceHandle.GetResourceIndex() == 
		spriteToErase.resourceHandle.GetResourceIndex());

	CHECK(spriteLoadedOvertop.resourceHandle.GetGeneration() ==
		spriteToErase.resourceHandle.GetGeneration() + 1);


	SDLite::Exit();
	Logger::EndSession();
}

TEST_CASE("SpriteAtlas::GetSpriteCount", "[atlas]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	SpriteAtlas spriteAtlas{};

	auto seriesPathsResult =
		ResourcePaths::SpriteDirectory("knight/fall_anim", std::less<std::string>{});
	REQUIRE_RESULT(seriesPathsResult);

	auto& seriesPaths = seriesPathsResult.GetValue();
	CHECK(seriesPaths.size() == 4);

	auto descriptors = test::MakeSpriteTestPackage(std::move(seriesPaths),
		"knight_fall_series");
	CHECK(descriptors.data.size() == 4);
	CHECK(descriptors.seriesName == "knight_fall_series");

	auto loadResult = spriteAtlas.LoadSprites(SDLite::Renderer(),
		std::move(descriptors));
	REQUIRE_RESULT(loadResult);

	const auto& sprites = loadResult.GetValue();
	for (const auto& sp : sprites)
	{
		CHECK(spriteAtlas.IsSpriteValid(sp));
		CHECK(spriteAtlas.GetSpriteInfo(sp).has_value());
	}

	size_t numSprites = 4;
	CHECK(spriteAtlas.GetSpriteCount() == numSprites);

	for (const auto& sp : sprites)
	{
		CHECK(spriteAtlas.EraseSprite(sp));
		CHECK(spriteAtlas.GetSpriteCount() == numSprites - 1);
		--numSprites;
	}

	SDLite::Exit();
	Logger::EndSession();
}

TEST_CASE("SpriteAtlas::Serialize/Deserialize", "[atlas][l]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	SpriteAtlas spriteAtlas{};

	// serialize
	auto fallSeriesPathsResult =
		ResourcePaths::SpriteDirectory("knight/fall_anim", std::less<std::string>{});
	REQUIRE_RESULT(fallSeriesPathsResult);

	auto& fallSeriesPaths = fallSeriesPathsResult.GetValue();
	CHECK(fallSeriesPaths.size() == 4);

	auto jumpSeriesPathsResult =
		ResourcePaths::SpriteDirectory("knight/jump_anim", std::less<std::string>{});
	REQUIRE_RESULT(jumpSeriesPathsResult);

	auto& jumpSeriesPaths = jumpSeriesPathsResult.GetValue();
	CHECK(jumpSeriesPaths.size() == 4);

	auto fallDescriptors = test::MakeSpriteTestPackage(std::move(fallSeriesPaths),
		"knight_fall_series");
	CHECK(fallDescriptors.data.size() == 4);
	CHECK(fallDescriptors.seriesName == "knight_fall_series");
	for (size_t i = 0; i < fallDescriptors.data.size(); ++i)
	{
		fallDescriptors.data[i].spriteName = std::format("knight_fall_{}", i);
	}

	auto jumpDescriptors = test::MakeSpriteTestPackage(std::move(jumpSeriesPaths),
		"knight_jump_series");
	CHECK(jumpDescriptors.data.size() == 4);
	CHECK(jumpDescriptors.seriesName == "knight_jump_series");
	for (size_t i = 0; i < jumpDescriptors.data.size(); ++i)
	{
		jumpDescriptors.data[i].spriteName = std::format("knight_jump_{}", i);
	}

	auto tempFallDescriptors = fallDescriptors;
	auto tempJumpDescriptors = jumpDescriptors;

	auto fallLoadResult = spriteAtlas.LoadSprites(SDLite::Renderer(),
		std::move(tempFallDescriptors));
	REQUIRE_RESULT(fallLoadResult);

	CHECK(spriteAtlas.GetSpriteCount() == 4);
	CHECK(spriteAtlas.HasSpriteSeries("knight_fall_series"));

	auto jumpLoadResult = spriteAtlas.LoadSprites(SDLite::Renderer(),
		std::move(tempJumpDescriptors));
	REQUIRE_RESULT(jumpLoadResult);

	CHECK(spriteAtlas.GetSpriteCount() == 8);
	CHECK(spriteAtlas.HasSpriteSeries("knight_jump_series"));

	auto package = spriteAtlas.Serialize();
	REQUIRE(package.spriteData.size() == 8);
	REQUIRE(package.seriesDefinitions.size() == 2);

	for (size_t i = 0; i < fallDescriptors.data.size(); ++i)
	{
		CHECK(core::Contains(package.spriteData, fallDescriptors.data[i]));
	}
	for (size_t i = 0; i < jumpDescriptors.data.size(); ++i)
	{
		CHECK(core::Contains(package.spriteData, jumpDescriptors.data[i]));
	}

	auto fallIt = core::FindIf(package.seriesDefinitions, [](const auto& def) {
		return def.seriesName == "knight_fall_series";
	});
	REQUIRE(fallIt != package.seriesDefinitions.end());
	REQUIRE(fallIt->spriteDataIndices.size() == 4);

	auto fallDescriptorSet = fallDescriptors.data | 
		std::views::transform([](const auto& desc) { return desc.spriteName; })
	| std::ranges::to<std::unordered_set>();

	for (const size_t idx : fallIt->spriteDataIndices)
	{
		REQUIRE(idx < package.spriteData.size());
		const auto& packageSpriteName = package.spriteData[idx].spriteName;
		CHECK(fallDescriptorSet.contains(packageSpriteName));
		fallDescriptorSet.erase(packageSpriteName);
	}

	auto jumpIt = core::FindIf(package.seriesDefinitions, [](const auto& def) {
		return def.seriesName == "knight_jump_series";
	});
	REQUIRE(jumpIt != package.seriesDefinitions.end());
	REQUIRE(jumpIt->spriteDataIndices.size() == 4);

	auto jumpDescriptorSet = jumpDescriptors.data |
		std::views::transform([](const auto& desc) { return desc.spriteName; })
	| std::ranges::to<std::unordered_set>();

	for (const size_t idx : jumpIt->spriteDataIndices)
	{
		REQUIRE(idx < package.spriteData.size());
		const auto& packageSpriteName = package.spriteData[idx].spriteName;
		CHECK(jumpDescriptorSet.contains(packageSpriteName));
		jumpDescriptorSet.erase(packageSpriteName);
	}

	SpriteAtlas spriteAtlas2{};
	
	auto deserializeResult = spriteAtlas2.Deserialize(SDLite::Renderer(), std::move(package));
	REQUIRE_RESULT(deserializeResult);

	CHECK(spriteAtlas2.GetSpriteCount() == 8);
	CHECK(spriteAtlas2.HasSpriteSeries("knight_fall_series"));
	CHECK(spriteAtlas2.HasSpriteSeries("knight_jump_series"));

	for (size_t i = 0; i < fallDescriptors.data.size(); ++i)
	{
		const auto& fallDescriptor = fallDescriptors.data[i];

		auto sp = spriteAtlas2.GetSprite(fallDescriptor.spriteName);
		CHECK(sp.resourceHandle.IsValid());
		CHECK(spriteAtlas2.IsSpriteValid(sp));

		CHECK(spriteAtlas2.GetSpriteSeriesMember("knight_fall_series", i) == sp);
		CHECK(spriteAtlas2.GetSpriteSeriesMemberIndex("knight_fall_series", sp) == i);
	}
	for (size_t i = 0; i < jumpDescriptors.data.size(); ++i)
	{
		const auto& jumpDescriptor = jumpDescriptors.data[i];

		auto sp = spriteAtlas2.GetSprite(jumpDescriptor.spriteName);
		CHECK(sp.resourceHandle.IsValid());
		CHECK(spriteAtlas2.IsSpriteValid(sp));

		CHECK(spriteAtlas2.GetSpriteSeriesMember("knight_jump_series", i) == sp);
		CHECK(spriteAtlas2.GetSpriteSeriesMemberIndex("knight_jump_series", sp) == i);
	}

	SDLite::Exit();
	Logger::EndSession();
}