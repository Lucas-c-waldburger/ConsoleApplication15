#include "../CatchMain.cpp"
#include "../../../core/ScopedInvoker.h"
#include "../../../atlas/SpriteAtlas.h"
#include "../../../file/FilePathUtility.h"
#include "../../../sdl/SDLite.h"


namespace {

//decltype(auto) InitSDL()
//{
//	Logger::StartSession();
//	auto status = SDLite::Start();
//	REQUIRE(status.Good());
//
//	return ScopedInvoker(&SDLite::Exit);
//}

//template <typename Fn>
//concept VecTransformFn = 


//template <typename T, typename U, typename...TArgs, typename...UArgs>
//std::vector<U, UArgs...> VecTransform(std::vector<T, TArgs...>&& vec, )

constexpr auto spriteIndexSort = [](const std::string& lhs, const std::string& rhs) {
	REQUIRE(!lhs.empty());
	REQUIRE(!rhs.empty());
	REQUIRE(std::isdigit(lhs.back()));
	REQUIRE(std::isdigit(rhs.back()));
	return lhs.back() < rhs.back();
};

SpriteDescriptorPackage MakePackage(const std::vector<std::string>& pathStrs, 
									std::string_view seriesName)
{
	SpriteDescriptorPackage package{};
	package.seriesName = seriesName;
	package.descriptors.resize(pathStrs.size());

	constexpr auto stripName = [](std::string_view pathSv) {
		auto path = fs::path(pathSv);
		REQUIRE(fs::exists(path));
		return path.stem().string();
	};

	std::transform(pathStrs.begin(), pathStrs.end(), package.descriptors.begin(), 
		[](auto&& pathStr) {
			return SpriteDescriptor{ 
				.spriteName = stripName(pathStr),
				.filepath = pathStr
			};
		});

	return package;
}

constexpr std::array<std::string_view, 4> kFallAnimSpriteNames = {
	"knight_fall_0", "knight_fall_1", "knight_fall_2", "knight_fall_3"
};

} // unnamed


TEST_CASE("Sprite Atlas Tests", "[atlas]")
{
	auto pathResult = ResourcePath::Sprite("knight/fall_anim/knight_fall_0.png");
	REQUIRE(pathResult.Success());

	const auto& path = pathResult.GetValue();

	SpriteDescriptor knightFallSpriteDescriptor{
		.spriteName = "knight_fall_0",
		.filepath = path
	};

	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());
	REQUIRE(SDLite::Renderer() != nullptr);

	auto createResult = SpriteAtlas::Create(SDLite::Renderer());
	REQUIRE_RESULT(createResult);
	auto& spriteAtlas = createResult.GetValue();

	CHECK(spriteAtlas.IsLoaded());
	CHECK(spriteAtlas.GetAtlasHandle().IsValid());
	REQUIRE(spriteAtlas.GetSourceTexture() != nullptr);

	{
	// Load single sprite
	auto loadResult = spriteAtlas.LoadSprite(
		SDLite::Renderer(), std::move(knightFallSpriteDescriptor)
	);
	REQUIRE_RESULT(loadResult);

	auto& sprite = loadResult.GetValue();
	CHECK(sprite.sourceAtlas == spriteAtlas.GetAtlasHandle());
	CHECK(sprite.plot.rect.w >= 128); 
	CHECK(sprite.plot.rect.h >= 128);
	CHECK(sprite.spriteIndex == 0);

	// validate it
	auto validateResult = spriteAtlas.ValidateSprite(sprite);
	CHECK_RESULT(validateResult);

	// get info
	const auto& info = spriteAtlas.GetSpriteInfo(sprite);
	CHECK(info != SpriteAtlas::kInvalidSpriteInfo);
	CHECK(info.spriteName == "knight_fall_0");
	CHECK(info.filepath == path);
	CHECK(info.seriesName.empty());
	CHECK(info.seriesIndex == kSizeMax);

	// retrieve it
	auto retrievedSprite = spriteAtlas.GetSprite("knight_fall_0");
	CHECK(retrievedSprite == sprite);
	}

	// load series on same atlas
	{
	auto seriesPathsResult = 
		ResourcePaths::SpriteDirectory("knight/fall_anim", spriteIndexSort);
	REQUIRE_RESULT(seriesPathsResult);

	auto& seriesPaths = seriesPathsResult.GetValue();
	CHECK(seriesPaths.size() == 4);

	auto package = MakePackage(std::move(seriesPaths), "fall_anim_series");
	CHECK(package.descriptors.size() == 4);
	CHECK(package.seriesName == "fall_anim_series");

	auto loadResult = spriteAtlas.LoadSprites(SDLite::Renderer(),
											  std::move(package));
	REQUIRE_RESULT(loadResult);

	auto& sprites = loadResult.GetValue();
	CHECK(sprites.size() == 4);
	
	for (size_t i = 0; i < sprites.size(); i++)
	{
		CHECK(sprites[i].sourceAtlas == spriteAtlas.GetAtlasHandle());
		CHECK(sprites[i].plot.rect.w >= 128);
		CHECK(sprites[i].plot.rect.h >= 128);
		CHECK(sprites[i].spriteIndex == i + 1); // <- since we already loaded 1 prev
	}
	
	// validate them
	for (const auto& sp : sprites)
	{
		auto validateResult = spriteAtlas.ValidateSprite(sp);
		CHECK_RESULT(validateResult);
	}

	// get info	
	for (size_t i = 0; i < sprites.size(); i++)
	{
		const auto& info = spriteAtlas.GetSpriteInfo(sprites[0]);
		CHECK(info != SpriteAtlas::kInvalidSpriteInfo);
		CHECK(info.spriteName == std::format("knight_fall_{}", i));
		CHECK(info.filepath == seriesPaths[i]);
		CHECK(info.seriesName == "knight_fall_series");
		CHECK(info.seriesIndex == i);
	}
	
	}

	SDLite::Exit();
}