#include <array>
#include "../CatchUtils.h"
#include "../../Fixtures.h"
#include "../../../systems/SerializationSystem.h"
#include "../../../systems/util/SerializationSystemUtils.h"
#include "../../../atlas/NewTextureRepository.h"
#include "../../../core/ScopedInvoker.h"
#include "../../../ecs/Ecs.h"
#include "../../../ecs/EntityPhysics.h"
#include "../../../serial/SerializationConcepts.h"
#include "../../../serial/EntitySerializer.h"
#include "../../../serial/EntityDeserializer.h"
#include "../../../serial/TextureRepositorySerializer.h"

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
	TRY(ResourcePath::Sprite("girl\\idle\\girl_idle_0.png"), sprite4Path);
	TRY(ResourcePath::Sprite("girl\\idle\\girl_idle_1.png"), sprite5Path);
	TRY(ResourcePath::Sprite("girl\\idle\\girl_idle_2.png"), sprite6Path);
	TRY(ResourcePath::Sprite("girl\\idle\\girl_idle_3.png"), sprite7Path);

	auto res1 = atlas.LoadSprites(SDLite::Renderer(), { .data = {
		{.filepath = sprite1Path },
		{.filepath = sprite2Path },
		{.filepath = sprite3Path }
	} });
	if (!res1.Success())
	{
		return res1.GetError();
	}

	return atlas.LoadSprites(SDLite::Renderer(), { .data = {
		{.filepath = sprite4Path },
		{.filepath = sprite5Path },
		{.filepath = sprite6Path },
		{.filepath = sprite7Path }
		}, .seriesName = "girl_idle" });
}

Result<Void> LoadTestFonts(FontAtlas& fontAtlas)
{
	TRY(ResourcePath::Font("GoNotoKurrent-Regular.ttf"), fontPath1);
	TRY(ResourcePath::Font("GoNotoKurrent-Bold.ttf"), fontPath2);

	TRY(fontAtlas.LoadFont(SDLite::Renderer(), 
		FontDescriptor{ .filepath = fontPath1, .fontSize = 24 }));
	TRY(fontAtlas.LoadFont(SDLite::Renderer(), 
		FontDescriptor{ .filepath = fontPath2, .fontSize = 32 }));

	return kVoid;
}

struct SkibidiComponent
{
	int i;
	float f;
	std::string str;
};
struct RizzComponent
{
	std::vector<int> vec;
	std::array<float, 3> arr;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SkibidiComponent, i, f, str)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RizzComponent, vec, arr)

} // unnamed

//TEST_CASE("Atlas Serialization", "[serial]")
//{
//	Logger::StartSession();
//	SDLite::Start();
//	SDL_PumpEvents();
//
//	TextureRepository repo{};
//	auto& spriteAtlas = repo.GetSpriteAtlas();
//	auto& fontAtlas = repo.GetFontAtlas();
//
//	// load sprites
//	auto spritesResult = LoadTestSprites(spriteAtlas);
//	REQUIRE_RESULT(spritesResult);
//
//	auto& sprites = spritesResult.GetValue();
//	CHECK(sprites.size() == 3);
//
//	// load font
//	auto fontPathResult = ResourcePath::Font("GoNotoKurrent-Regular.ttf");
//	REQUIRE_RESULT(fontPathResult);
//
//	auto fontLoadResult = fontAtlas.LoadFont(SDLite::Renderer(),
//		FontDescriptor{ .filepath = fontPathResult.GetValue(), .fontSize = 32 });
//	REQUIRE_RESULT(fontLoadResult);
//
//	// serialize atlases
//	nlohmann::json j;
//	auto& atlasesJson = j[kTextureAtlasesKey];
//
//	auto serializationResult = 
//		util::TextureRepositorySerializationHelper::SerializeAtlases(repo, atlasesJson);
//	REQUIRE_RESULT(serializationResult);
//
//	CHECK(atlasesJson.contains(kSpriteAtlasesKey));
//	CHECK(atlasesJson.at(kSpriteAtlasesKey).is_array());
//	CHECK(atlasesJson.at(kSpriteAtlasesKey).size() == 1);
//
//	CHECK(atlasesJson.contains(kGlyphAtlasesKey));
//	CHECK(atlasesJson.at(kGlyphAtlasesKey).is_array());
//	CHECK(atlasesJson.at(kGlyphAtlasesKey).size() == 1);
//
//	std::ofstream file(MakeJsonTestPath("atlases.json"));
//	REQUIRE(file);
//
//	file << std::setw(4) << j;
//
//	Logger::EndSession();
//	SDLite::Exit();
//}

TEST_CASE("Entity Serialization", "[serial]")
{
	const auto texturesPath = MakeJsonTestPath("textures.json");
	const auto entitiesPath = MakeJsonTestPath("entities.json");

	{ // SERIALIZE
	auto sceneResult = SceneFixture::GetInstance();
	REQUIRE(sceneResult.Success());
	REQUIRE(sceneResult.GetValue());

	auto& scene = sceneResult.GetValue();

	auto loadSpritesResult = LoadTestSprites(scene->GetTextureRepository().GetSpriteAtlas());
	CHECK(loadSpritesResult.Success());

	auto fontLoadResult = LoadTestFonts(scene->GetTextureRepository().GetFontAtlas());
	CHECK(fontLoadResult.Success());

	// serialize textures
	auto txSerializeResult = TextureRepositorySerializer::Serialize(
		texturesPath, scene->GetTextureRepository()
	);
	CHECK(txSerializeResult.Success());

	auto e = ECS::CreateEntity();

	e.AddComponent(Transform{ .position = { 0.0f, 0.0f }, .rotation = 45.0f, .scale = { 2.0f, 5.0f } });
	e.AddComponent(CameraTarget{ .offset = { 2.5f, -39.5f }, .followSpeed = 3.0f, .stopRadius = 1.0f});
	e.AddComponent(Tags{ .tags = { "Player", "Controllable" } });
	e.AddComponent(GameControllerState{});

	auto sprite = scene->GetTextureRepository().GetSpriteAtlas().GetSprite("girl_idle_0");
	REQUIRE(sprite.resourceHandle.IsValid());
	e.AddComponent(SpriteRenderableComponent{ 
		.sprite = sprite, 
		.profile = {
			.drawOrder = 5,
			.mods = TextureMods{ .color = { 255, 0, 0 }, .alpha = 128 },
			.flip = SDL_FLIP_HORIZONTAL,
			.offset = { 10.0f, -5.0f },
			.debugDraw = DebugDrawSet{ .collider = { .on = true, .color = { 0, 255, 0 } } },
			.isOverlay = true,
			.parallaxFactor = 0.5f,
			.anchor = {
				.scale = Anchor::TopLeft,
				.rotation = Anchor::Center
			}
		}
	});

	e.AddComponent(SpriteAnimationComponent{ 
		.spriteSeriesName = "girl_idle", 
		.index = { .current = 1, .max = 2 } 
	});

	e.SetEventProduction<events::ContactCollisionBegin>(false);
	e.SetComponentVisibility<CameraTarget>(false);

	auto rels = e.GetRelations();
	auto ch = rels.AddChild(); 
	ch.AddComponent(Tags{ .tags = { "ManualChild" } });

	auto phys = e.GetPhysics(scene->GetWorld());
	phys.AddBody(B2Body::Type::Dynamic, { 15.0f, 25.0f });
	phys.AddColliderBox({ 15.0f, 25.0f });
	phys.AddColliderCircle(33.0f);

	// serialize entities
	EntitySerializer eSerializer{ scene->GetTextureRepository() };
	auto eSerializeResult = eSerializer.SerializeEntities(entitiesPath);
	CHECK(eSerializeResult.Success());
	} //


	// DESERIALIZE
	{
	REQUIRE(ECS::GetAllActiveEntities().empty());

	auto sceneResult = SceneFixture::GetInstance();
	REQUIRE(sceneResult.Success());
	REQUIRE(sceneResult.GetValue());

	auto& scene = sceneResult.GetValue();

	// deserialize textures
	auto txErrors = TextureRepositorySerializer::Deserialize(
		texturesPath, scene->GetTextureRepository(), scene->GetRenderer());

	if (!txErrors.empty())
	{
		for (const auto& err : txErrors)
		{
			WARN(err.GetMessage());
		}
	}

	CHECK(txErrors.empty());

	// deserialize entities
	auto eErrors = EntityDeserializer{ 
		scene->GetWorld(), scene->GetTextureRepository(), scene->GetSystem<SDLInputSystem>() 
	}.DeserializeEntities(entitiesPath);

	if (!eErrors.empty())
	{
		for (const auto& err : eErrors)
		{
			WARN(err.GetMessage());
		}
	}

	CHECK(eErrors.empty());

	auto entities = ECS::GetAllEntitiesWith<Exclude<Parent>>();
	REQUIRE(entities.size() == 1);	
	auto& e = entities.front();

	SECTION("Check Transform")
	{
		REQUIRE(e.HasComponent<Transform>());
		const auto& tf = e.GetComponent<Transform>();
		CHECK(EqualsWithTolerance(tf.position.x, 15.0f));
		CHECK(EqualsWithTolerance(tf.position.y, 25.0f));
		CHECK(EqualsWithTolerance(tf.rotation, 45.0f));
		CHECK(EqualsWithTolerance(tf.scale.x, 2.0f));
		CHECK(EqualsWithTolerance(tf.scale.y, 5.0f));
	}

	SECTION("Check CameraTarget")
	{
		REQUIRE(e.HasComponent<CameraTarget>());
		const auto& camTarget = e.GetComponent<CameraTarget>();
		CHECK(EqualsWithTolerance(camTarget.offset.x, 2.5f));
		CHECK(EqualsWithTolerance(camTarget.offset.y, -39.5f));
		CHECK(EqualsWithTolerance(camTarget.followSpeed, 3.0f));
		CHECK(EqualsWithTolerance(camTarget.stopRadius, 1.0f));
	}

	SECTION("Check Tags")
	{
		REQUIRE(e.HasComponent<Tags>());
		const auto& tags = e.GetComponent<Tags>().tags;
		CHECK(tags.size() == 2);
		CHECK(tags.contains("Player"));
		CHECK(tags.contains("Controllable"));
	}
	
	SECTION("Check GameControllerState")
	{
		REQUIRE(e.HasComponent<GameControllerState>());
		const auto& gcState = e.GetComponent<GameControllerState>();
		CHECK(gcState.joystickID == -1);
	}

	SECTION("Check SpriteRenderableComponent")
	{
		REQUIRE(e.HasComponent<SpriteRenderableComponent>());
		const auto& spriteRenderable = e.GetComponent<SpriteRenderableComponent>();
		CHECK(spriteRenderable.sprite.resourceHandle.IsValid());
		CHECK(spriteRenderable.profile.drawOrder == 5);
		CHECK(spriteRenderable.profile.mods.color.r == 255);
		CHECK(spriteRenderable.profile.mods.color.g == 0);
		CHECK(spriteRenderable.profile.mods.color.b == 0);
		CHECK(spriteRenderable.profile.mods.alpha == 128);
		CHECK(spriteRenderable.profile.flip == SDL_FLIP_HORIZONTAL);
		CHECK(EqualsWithTolerance(spriteRenderable.profile.offset.x, 10.0f));
		CHECK(EqualsWithTolerance(spriteRenderable.profile.offset.y, -5.0f));
		CHECK(spriteRenderable.profile.debugDraw.collider.on);
		CHECK(spriteRenderable.profile.debugDraw.collider.color.r == 0);
		CHECK(spriteRenderable.profile.debugDraw.collider.color.g == 255);
		CHECK(spriteRenderable.profile.debugDraw.collider.color.b == 0);
		CHECK(spriteRenderable.profile.isOverlay);
		CHECK(EqualsWithTolerance(spriteRenderable.profile.parallaxFactor, 0.5f));
		CHECK(spriteRenderable.profile.anchor.scale == Anchor::TopLeft);
		CHECK(spriteRenderable.profile.anchor.rotation == Anchor::Center);
	}

	SECTION("Check SpriteAnimationComponent")
	{
		REQUIRE(e.HasComponent<SpriteAnimationComponent>());
		const auto& anim = e.GetComponent<SpriteAnimationComponent>();
		CHECK(anim.index.current == 1);
		CHECK(anim.index.max == 2);
		CHECK(anim.spriteSeriesName == "girl_idle");
	}

	SECTION("Check RigidBody")
	{
		REQUIRE(e.HasComponent<RigidBody>());
		const auto& rb = e.GetComponent<RigidBody>();
		const auto& body = rb.body.GetData();
		CHECK(body.IsValid());
		CHECK(body.GetBodyType() == B2Body::Type::Dynamic);
		CHECK(EqualsWithTolerance(body.GetPosition().x, 15.0f));
		CHECK(EqualsWithTolerance(body.GetPosition().y, 25.0f));
	}

	SECTION("Check Collider")
	{
		REQUIRE(e.HasComponent<Collider>());
		const auto& collider = e.GetComponent<Collider>();
		const auto& shape = collider.shape.GetData();
		CHECK(shape.IsValid());
		CHECK(shape.GetShapeType() == B2Shape::Type::Polygon);
	}

	SECTION("Check Flags")
	{
		CHECK_FALSE(e.ShouldProduceEvent<events::ContactCollisionBegin>());
		CHECK_FALSE(e.GetComponentVisibility<CameraTarget>());
	}

	SECTION("Check Relations")
	{
		auto rels = e.GetRelations();
		REQUIRE(rels.HasChildren());

		auto children = rels.GetChildren();
		REQUIRE(children.size() == 2); // one real child, one proxy child

		std::vector<Entity> manualChildren;
		std::vector<Entity> colliderChildren;
		for (const auto& child : children)
		{
			if (child.HasComponent<Tags>() && 
				child.GetComponent<Tags>().tags.contains("ManualChild"))
			{
				manualChildren.push_back(child);
			}
			if (child.HasComponent<Collider>())
			{
				colliderChildren.push_back(child);
			}
		}

		REQUIRE(manualChildren.size() == 1);
		REQUIRE(colliderChildren.size() == 1);

		const auto& colliderChild = colliderChildren.front();
		const auto& collider = colliderChild.GetComponent<Collider>();
		const auto& shape = collider.shape.GetData();
		
		CHECK(shape.IsValid());
		CHECK(shape.GetShapeType() == B2Shape::Type::Circle);
		CHECK(EqualsWithTolerance(shape.GetAs<B2CircleShape>().GetRadius(), 33.0f));

		REQUIRE(e.HasComponent<RigidBody>());
		const auto& parentRb = e.GetComponent<RigidBody>();
		const auto& parentBody = parentRb.body.GetData();

		CHECK(parentBody.OwnsShape(shape.GetHandle()));
	}
	}
}

TEST_CASE("UserComponent Serialization", "[serial]")
{
	static_assert(HasToJson<SkibidiComponent>);
	static_assert(HasToJson<RizzComponent>);
	static_assert(HasFromJson<SkibidiComponent>);
	static_assert(HasFromJson<RizzComponent>);

	auto e = ECS::CreateEntity();

	ScopedInvoker destroyEnt([&e]() {
		e.Destroy();
	});

	const bool skibidiRegistered = ECS::RegisterComponent<SkibidiComponent>("Skibidi");
	CHECK(skibidiRegistered);

	const bool rizzRegistered = ECS::RegisterComponent<RizzComponent>();
	CHECK(rizzRegistered);

	e.AddComponent(SkibidiComponent{ .i = 42, .f = 3.14f, .str = "Skibidi bop yes yes yes" });
	REQUIRE(e.HasComponent<SkibidiComponent>());

	e.AddComponent(RizzComponent{ .vec = { 2, 62, 64, 4, 56 }, .arr = { 0.1f, 1.2f, 33.3f } });
	REQUIRE(e.HasComponent<RizzComponent>());

	auto userCmpJsonPath = MakeJsonTestPath("user_components.json");

	// serialize
	{
	nlohmann::json j;
	ECS::SerializeUserComponents(j, e);

	std::ofstream file(userCmpJsonPath);
	REQUIRE(file);
	file << j.dump(4);
	file.close();
	}

	// deserialize
	std::ifstream file(userCmpJsonPath);
	REQUIRE(file);

	nlohmann::json j;
	try
	{
		j = nlohmann::json::parse(file);
	}
	catch (const nlohmann::json::parse_error& err)
	{
		CAPTURE(err.what());
		REQUIRE(false);
	}

	REQUIRE(j.contains("Skibidi"));
	REQUIRE(j.contains(typeid(RizzComponent).name()));

	e.RemoveComponent<SkibidiComponent>();
	CHECK_FALSE(e.HasComponent<SkibidiComponent>());
	e.RemoveComponent<RizzComponent>();
	CHECK_FALSE(e.HasComponent<RizzComponent>());

	auto deserializeResult = ECS::DeserializeUserComponents(j, e);	
	CHECK(deserializeResult.Success());

	REQUIRE(e.HasComponent<SkibidiComponent>());
	REQUIRE(e.HasComponent<RizzComponent>());

	const auto& deserializedSkibidi = e.GetComponent<SkibidiComponent>();
	const auto& deserializedRizz = e.GetComponent<RizzComponent>();

	CHECK(deserializedSkibidi.i == 42);
	CHECK(EqualsWithTolerance(deserializedSkibidi.f, 3.14f));
	CHECK(deserializedSkibidi.str == "Skibidi bop yes yes yes");

	CHECK(deserializedRizz.vec == std::vector<int>{ 2, 62, 64, 4, 56 });
	CHECK(deserializedRizz.arr[0] == 0.1f);
	CHECK(deserializedRizz.arr[1] == 1.2f);
	CHECK(deserializedRizz.arr[2] == 33.3f);	

	//auto sceneResult = SceneFixture::GetInstance();
	//REQUIRE(sceneResult.Success());
	//REQUIRE(sceneResult.GetValue());
	//auto& scene = sceneResult.GetValue();

	//auto e = ECS::CreateEntity();
	//e.AddComponent(Transform{ .position = { 10.0f, 20.0f }, .rotation = 45.0f, .scale = { 2.0f, 5.0f } });
	//e.AddComponent(CameraTarget{ .followSpeed = 3.0f, .stopRadius = 1.0f });
	//e.AddComponent(Tags{ .tags = { "Player", "Controllable" } });
	//e.AddComponent(GameControllerState{});
	//auto serializeResult = EntitySerializer{ scene->GetTextureRepository() }.SerializeEntities(
	//	MakeJsonTestPath("user_components.json"));
	//CHECK(serializeResult.Success());
	//auto deserializeErrors = EntityDeserializer{ 
	//	scene->GetWorld(), scene->GetTextureRepository(), scene->GetSystem<SDLInputSystem>() 
	//}.DeserializeEntities(MakeJsonTestPath("user_components.json"));
	//CHECK(deserializeErrors.empty());
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