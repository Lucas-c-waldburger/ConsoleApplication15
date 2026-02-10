#include <array>
#include "../CatchUtils.h"
#include "../../../systems/SerializationSystem.h"
#include "../../../systems/util/SerializationSystemUtils.h"
#include "../../../atlas/NewTextureRepository.h"
#include "../../../ecs/Ecs.h"

// TODO : Fix since new texture repository
namespace {

static constexpr std::string_view kSprite1Name = "knight_walk_0";
static constexpr std::string_view kSprite2Name = "knight_jump_0";
static constexpr std::string_view kSprite3Name = "knight_fall_0";

std::string MakeJsonTestPath(std::string_view jsonFilename)
{
	namespace fs = std::filesystem;

	auto path = FilePathUtility::GetRootPath() /
		fs::path("test/catch/test_json") / fs::path(jsonFilename);

	return path.string();
}

Result<std::vector<Sprite>> LoadTestSprites(SpriteAtlas& atlas)
{
	TRY(ResourcePath::Sprite("knight\\walk_anim\\knight_walk_0.png"), sprite1Path);
	TRY(ResourcePath::Sprite("knight\\jump_anim\\knight_jump_0.png"), sprite2Path);
	TRY(ResourcePath::Sprite("knight\\fall_anim\\knight_fall_0.png"), sprite3Path);

	return atlas.LoadSprites(SDLite::Renderer(), { .data = {
		{.filepath = sprite1Path },
		{.filepath = sprite2Path },
		{.filepath = sprite3Path }
	} });
}


} // unnamed

TEST_CASE("Atlas Serialization", "[serial]")
{
	Logger::StartSession();
	SDLite::Start();
	SDL_PumpEvents();

	TextureRepository repo{};
	auto& spriteAtlas = repo.GetSpriteAtlas();
	auto& fontAtlas = repo.GetFontAtlas();

	// load sprites
	auto spritesResult = LoadTestSprites(spriteAtlas);
	REQUIRE_RESULT(spritesResult);

	auto& sprites = spritesResult.GetValue();
	CHECK(sprites.size() == 3);

	// load font
	auto fontPathResult = ResourcePath::Font("GoNotoKurrent-Regular.ttf");
	REQUIRE_RESULT(fontPathResult);

	auto fontLoadResult = fontAtlas.LoadFont(SDLite::Renderer(),
		FontDescriptor{ .filepath = fontPathResult.GetValue(), .fontSize = 32 });
	REQUIRE_RESULT(fontLoadResult);

	// serialize atlases
	nlohmann::json j;
	auto& atlasesJson = j[kTextureAtlasesKey];

	auto serializationResult = 
		util::TextureRepositorySerializationHelper::SerializeAtlases(repo, atlasesJson);
	REQUIRE_RESULT(serializationResult);

	CHECK(atlasesJson.contains(kSpriteAtlasesKey));
	CHECK(atlasesJson.at(kSpriteAtlasesKey).is_array());
	CHECK(atlasesJson.at(kSpriteAtlasesKey).size() == 1);

	CHECK(atlasesJson.contains(kGlyphAtlasesKey));
	CHECK(atlasesJson.at(kGlyphAtlasesKey).is_array());
	CHECK(atlasesJson.at(kGlyphAtlasesKey).size() == 1);

	std::ofstream file(MakeJsonTestPath("atlases.json"));
	REQUIRE(file);

	file << std::setw(4) << j;

	Logger::EndSession();
	SDLite::Exit();
}


//TEST_CASE("Atlas Deserialization", "[serial]")
//{
//	Logger::StartSession();
//	SDLite::Start();
//	SDL_PumpEvents();
//
//	TextureRepository repo{};
//
//	std::ifstream file(MakeJsonTestPath("atlases.json"));
//	REQUIRE(file);
//
//	nlohmann::json j;
//	try
//	{
//		j = nlohmann::json::parse(file);
//	}
//	catch (const nlohmann::json::parse_error& err)
//	{
//		CAPTURE(err.what());
//		REQUIRE(false);
//	}
//	
//	REQUIRE(j.contains(kTextureAtlasesKey));
//
//	auto handleHashMapResult =
//		util::TextureRepositorySerializationHelper::DeserializeAtlases(
//			SDLite::Renderer(), repo, j.at(kTextureAtlasesKey));
//	REQUIRE_RESULT(handleHashMapResult);
//
//	const auto& handleHashMap = handleHashMapResult.GetValue();
//	CHECK(handleHashMap.size() == 2);
//
//	for (const auto& [hash, handle] : handleHashMap)
//	{
//		CHECK(hash > 0);
//		CHECK(repo.HasAtlas(handle));
//	}
//
//	const auto& cRepo = repo;
//
//	REQUIRE(cRepo.GetAtlasVector<SpriteAtlasTexture>().size() == 1);
//	const auto& spriteAtlasHandle = cRepo.GetAtlasVector<SpriteAtlasTexture>().front().GetHandle();
//
//	REQUIRE(cRepo.GetAtlasVector<FontAtlasTexture>().size() == 1);
//	const auto& glyphAtlasHandle = cRepo.GetAtlasVector<FontAtlasTexture>().front().GetHandle();
//
//	auto* spriteAtlas = repo.GetAtlas<SpriteAtlasTexture>(spriteAtlasHandle);
//	REQUIRE(spriteAtlas);
//	CHECK(spriteAtlas->GetTextureSize() == 1024);
//
//	auto* glyphAtlas = repo.GetAtlas<FontAtlasTexture>(glyphAtlasHandle);
//	REQUIRE(glyphAtlas);
//	CHECK(glyphAtlas->GetTextureSize() == 512);
//
//	// check sprites
//	auto sprite1PathResult = ResourcePath::Sprite("knight\\walk_anim\\knight_walk_0.png");
//	REQUIRE_RESULT(sprite1PathResult);
//	auto sprite2PathResult = ResourcePath::Sprite("knight\\jump_anim\\knight_jump_0.png");
//	REQUIRE_RESULT(sprite2PathResult);
//	auto sprite3PathResult = ResourcePath::Sprite("knight\\fall_anim\\knight_fall_0.png");
//	REQUIRE_RESULT(sprite3PathResult);
//
//	auto sprite1 = spriteAtlas->GetSprite(kSprite1Name);
//	{
//		CHECK(sprite1.resourceHandle == spriteAtlasHandle);
//		CHECK(sprite1.spriteIndex == 0);
//		CHECK(sprite1.plot.rect.w > 0);
//		CHECK(sprite1.plot.rect.h > 0);
//
//		auto sprite1Info = spriteAtlas->GetSpriteInfo(sprite1);
//		REQUIRE(sprite1Info.has_value());
//
//		auto [plot, name, path, series, seriesIdx] = *sprite1Info;
//		CHECK(plot == sprite1.plot);
//		CHECK(name == kSprite1Name);
//		CHECK(path == sprite1PathResult.GetValue());
//		CHECK(series.empty());
//		CHECK(seriesIdx == kSizeMax);
//	}
//
//	auto sprite2 = spriteAtlas->GetSprite(kSprite2Name);
//	{
//		CHECK(sprite2.resourceHandle == spriteAtlasHandle);
//		CHECK(sprite2.spriteIndex == 1);
//		CHECK(sprite2.plot.rect.w > 0);
//		CHECK(sprite2.plot.rect.h > 0);
//
//		auto sprite2Info = spriteAtlas->GetSpriteInfo(sprite2);
//		REQUIRE(sprite2Info.has_value());
//
//		auto [plot, name, path, series, seriesIdx] = *sprite2Info;
//		CHECK(plot == sprite2.plot);
//		CHECK(name == kSprite2Name);
//		CHECK(path == sprite2PathResult.GetValue());
//		CHECK(series.empty());
//		CHECK(seriesIdx == kSizeMax);
//	}
//
//	auto sprite3 = spriteAtlas->GetSprite(kSprite3Name);
//	{
//		CHECK(sprite3.resourceHandle == spriteAtlasHandle);
//		CHECK(sprite3.spriteIndex == 2);
//		CHECK(sprite3.plot.rect.w > 0);
//		CHECK(sprite3.plot.rect.h > 0);
//
//		auto sprite3Info = spriteAtlas->GetSpriteInfo(sprite3);
//		REQUIRE(sprite3Info.has_value());
//
//		auto [plot, name, path, series, seriesIdx] = *sprite3Info;
//		CHECK(plot == sprite3.plot);
//		CHECK(name == kSprite3Name);
//		CHECK(path == sprite3PathResult.GetValue());
//		CHECK(series.empty());
//		CHECK(seriesIdx == kSizeMax);
//	}
//
//	// check glyph atlas
//	auto fontPathResult = ResourcePath::Font("GoNotoKurrent-Regular.ttf");
//	REQUIRE_RESULT(fontPathResult);
//
//	const auto& fontDescriptor = glyphAtlas->GetFontDescriptor();
//	CHECK(fontDescriptor.fontName == "GoNotoKurrent-Regular");
//	CHECK(fontDescriptor.filepath == fontPathResult.GetValue());
//	CHECK(fontDescriptor.fontSize == 32);
//	CHECK(fontDescriptor.fontHeight > 0);
//
//	Logger::EndSession();
//	SDLite::Exit();
//}
//
//TEST_CASE("Serialization System Tests", "[serial][system]")
//{
//	Logger::StartSession();
//	SDLite::Start();
//	SDL_PumpEvents();
//
//	TextureRepository repo{};
//
//	auto spriteAtlasResult = repo.CreateAtlas<SpriteAtlasTexture>(SDLite::Renderer());
//	REQUIRE_RESULT(spriteAtlasResult);
//	REQUIRE(spriteAtlasResult.GetValue());
//
//	auto& spriteAtlas = *spriteAtlasResult.GetValue();
//
//	auto spritesResult = LoadTestSprites(spriteAtlas);
//	REQUIRE_RESULT(spritesResult);
//
//	auto& sprites = spritesResult.GetValue();
//	CHECK(sprites.size() == 3);
//
//	SerializationSystem serialSystem;
//	
//	auto e1 = ECS::CreateEntity();
//	auto e2 = ECS::CreateEntity();
//	auto e3 = ECS::CreateEntity();
//
//	REQUIRE(e1.IsValid());
//	REQUIRE(e2.IsValid());
//	REQUIRE(e3.IsValid());
//
//	auto& spriteCmp1 = e1.AddComponent<SpriteRenderableComponent>();
//	spriteCmp1.sprite = sprites[0];
//	spriteCmp1.profile.anchor.scale = Anchor::TopRight;
//	spriteCmp1.profile.anchor.rotation = Anchor::BottomRight;
//
//	auto& spriteCmp2 = e2.AddComponent<SpriteRenderableComponent>();
//	spriteCmp2.sprite = sprites[1];
//	spriteCmp2.profile.flip = SDL_FLIP_HORIZONTAL;
//	spriteCmp2.profile.drawOrder = 2222;
//
//	auto& spriteCmp3 = e3.AddComponent<SpriteRenderableComponent>();
//	spriteCmp3.sprite = sprites[2];
//	spriteCmp3.profile.debugDraw.boundingBox.color = { 3, 3, 3, 3 };
//	spriteCmp3.profile.mods.blend = SDL_BLENDMODE_ADD;
//	spriteCmp3.profile.offset = { 3.3f, 333.3f };
//
//	//serialSystem.SerializeState
//
//	Logger::EndSession();
//	SDLite::Exit();
//}