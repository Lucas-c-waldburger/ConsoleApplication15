#include "../CatchUtils.h"
#include "../test_utils/ImageUtilities.h"
#include "../test_utils/AtlasTestUtils.h"
#include "../../../file/FilePathUtility.h"
#include "../../Fixtures.h"
#include "../../../atlas/NewTextureRepository.h"
#include "../../../ecs/Ecs.h"
#include <random>

template <typename T>
T GetRandom(T min, T max)
{
	static thread_local std::mt19937 rng{ std::random_device{}() };

	std::uniform_real_distribution<float> dist(
		static_cast<float>(min), 
		static_cast<float>(max)
	);

	return static_cast<T>(dist(rng));
}
template <typename T>
T GetRandomCentered(T magnitude)
{
	return GetRandom(-magnitude, magnitude);
}


TEST_CASE("System handles single sprite", "[rendering][system]")
{
	auto fixtureResult = SceneFixture::GetInstance();
	REQUIRE_RESULT(fixtureResult);
	auto& fixture = fixtureResult.GetValue();

	auto& textureRepo = fixture->GetTextureRepository();
	auto& spriteAtlas = textureRepo.GetSpriteAtlas();

	auto spritePathResult = ResourcePath::Sprite("knight_new/idle/Idle_000.png");
	REQUIRE_RESULT(spritePathResult);

	SpriteDescriptor desc{
		.filepath = spritePathResult.GetValue(),
	};

	auto spriteLoadResult = spriteAtlas.LoadSprite(SDLite::Renderer(), std::move(desc));
	REQUIRE_RESULT(spriteLoadResult);

	auto sprite = spriteLoadResult.GetValue();

	CHECK(sprite.plot.rect.w > 0);
	CHECK(sprite.plot.rect.h > 0);
	CHECK(sprite.resourceHandle.IsValid());
	CHECK(textureRepo.GetSourceTexture(sprite.resourceHandle) != nullptr);

	const auto spriteName = spriteAtlas.GetSpriteInfo<&SpriteInfo::spriteName>(sprite);
	CHECK(spriteName.has_value());
	CHECK(*spriteName == "Idle_000");

	auto entity = ECS::CreateEntity();
	REQUIRE(entity.IsValid());

	entity.AddComponent(SpriteRenderableComponent{
		.sprite = sprite,
		.profile = { .debugDraw = { .boundingBox = { .on = false }}}
	});
	entity.AddComponent(Transform{
		.position = SDLite::Window().GetLocalCenter<SDL_FPoint>()
	});

	SDL_PumpEvents();
	SDLite::Renderer().Clear(SDLite::kColorWhite);

	auto& cam = fixture->GetSystem<CameraSystem>().GetCamera();

	fixture->GetSystem<NewRenderSystem>().Update(SDLite::Renderer(), cam, textureRepo);

	SDLite::Renderer().Show();

	auto imgWriteResult = test::WriteFrameBufferToPNG(SDLite::Renderer(), 
		"render_system_single_sprite_NEW.png");

	REQUIRE_RESULT(imgWriteResult);
}

TEST_CASE("System handles sprites and glyphs", "[rendering][system]")
{
	auto fixtureResult = SceneFixture::GetInstance();
	REQUIRE_RESULT(fixtureResult);
	auto& fixture = fixtureResult.GetValue();

	auto& textureRepo = fixture->GetTextureRepository();
	auto& spriteAtlas = textureRepo.GetSpriteAtlas();

	auto airbornePathsResult =
		ResourcePaths::SpriteDirectory("knight_new/airborne", std::less<std::string>{});
	REQUIRE_RESULT(airbornePathsResult);

	auto& airbornePaths = airbornePathsResult.GetValue();
	CHECK(airbornePaths.size() == 12);

	auto package = test::MakeSpriteTestPackage(std::move(airbornePaths), "airborne_series");
	CHECK(package.data.size() == 12);
	CHECK(package.seriesName == "airborne_series");

	std::vector<Entity> entities;
	for (size_t i = 0; i < package.data.size(); i++)
	{
		auto spriteResult = spriteAtlas.LoadSprite(
			SDLite::Renderer(), std::move(package.data[i]));
		REQUIRE_RESULT(spriteResult);

		size_t nEntites = GetRandom(1, 4);
		for (size_t i = 0; i < nEntites; i++)
		{
			auto& entity = entities.emplace_back(ECS::CreateEntity());
			REQUIRE(entity.IsValid());

			entity.AddComponent(Transform{
				.position = {
					GetRandom(30.0f, static_cast<float>(SDLite::Window().GetSize().w - 30)),
					GetRandom(30.0f, static_cast<float>(SDLite::Window().GetSize().h - 30))					
				},
				.rotation = GetRandom(0.0f, 360.0f),
				.scale = {
					GetRandom(0.5f, 2.5f),
					GetRandom(0.5f, 2.5f)
				}
			});

			entity.AddComponent(SpriteRenderableComponent{
				.sprite = spriteResult.GetValue(),
				.profile = {.debugDraw = {.boundingBox = {.on = false }}}
			});
		}
	}

	SDL_PumpEvents();
	SDLite::Renderer().Clear(SDLite::kColorWhite);

	auto& cam = fixture->GetSystem<CameraSystem>().GetCamera();

	fixture->GetSystem<NewRenderSystem>().Update(SDLite::Renderer(), cam, textureRepo);

	SDLite::Renderer().Show();

	auto imgWriteResult = test::WriteFrameBufferToPNG(SDLite::Renderer(),
		"render_system_many_sprites_different_atlases_NEW.png");

	REQUIRE_RESULT(imgWriteResult);
}