#include <ranges>
#include <format>
#include "../CatchUtils.h"
#include "../../Fixtures.h"
#include "../../../file/FilePathUtility.h"
#include "../../../ecs/Ecs.h"


TEST_CASE("SpriteAnimationSystem correctly updates sprites", "[animation][system]")
{
	// Load sprite series
	auto fixtureResult = SceneFixture::GetInstance();
	REQUIRE_RESULT(fixtureResult);
	auto& fixture = fixtureResult.GetValue();

	auto seriesPathsResult = ResourcePaths::SpriteDirectory(
		"slash_effect/Slash 1/color1/Frames"
	);
	REQUIRE_RESULT(seriesPathsResult);

	auto descriptors = seriesPathsResult.GetValue() 
	| std::views::transform([](auto&& path) {
		return SpriteDescriptor{ .filepath = std::move(path) };
	}) | std::ranges::to<std::vector>();

	REQUIRE(descriptors.size() == 9);

	auto& spriteAtlas = fixture->GetTextureRepository().GetSpriteAtlas();
	auto spritesResult = spriteAtlas.LoadSprites(fixture->GetRenderer(), { 
		.data = std::move(descriptors), 
		.seriesName = "sword_slash" 
	});
	REQUIRE_RESULT(spritesResult);
	auto& sprites = spritesResult.GetValue();

	REQUIRE(sprites.size() == 9);

	// validate sprites
	constexpr std::string_view spriteNameFmt = "Slash_color1_frame{}";
	for (size_t i = 1; i <= 9; i++)
	{
		const auto& sprite = sprites[i - 1];

		REQUIRE(spriteAtlas.IsSpriteValid(sprite));

		auto info = spriteAtlas.GetSpriteInfo<&SpriteInfo::spriteName,
											  &SpriteInfo::seriesName,
											  &SpriteInfo::seriesIndex>(sprite);
		REQUIRE(info.has_value());

		const auto& [spriteName, seriesName, seriesIdx] = *info;

		CHECK(spriteName == std::format(spriteNameFmt, i));
		CHECK(seriesName == "sword_slash");
		CHECK(seriesIdx == i - 1);
	}

	// prep entity
	auto entity = ECS::CreateEntity();
	REQUIRE(entity.IsValid());

	entity.AddComponent(Transform{
		.position = SDLite::Window().GetLocalCenter<SDL_FPoint>()
	});
	entity.AddComponent<SpriteRenderableComponent>();

	SECTION("NeedsAnimationUpdate applied when adding SpriteAnimationComponent")
	{
		CHECK_FALSE(entity.HasComponent<NeedsAnimationUpdate>());

		entity.AddComponent<SpriteAnimationComponent>();

		CHECK(entity.HasComponent<NeedsAnimationUpdate>());
	}

	SECTION("Mutating SpriteAnimationComponent updates SpriteRenderableComponent")
	{
		REQUIRE(entity.HasComponent<SpriteRenderableComponent>());
		CHECK_FALSE(entity.GetComponent<SpriteRenderableComponent>()
			.sprite.resourceHandle.IsValid());

		entity.AddComponent(SpriteAnimationComponent{
			.spriteSeriesName = "sword_slash",
			.index = { .current = 0 }
		});

		CHECK(entity.HasComponent<NeedsAnimationUpdate>());

		fixture->StepGameLoop(1);

		REQUIRE(entity.HasComponent<SpriteRenderableComponent>());
		const auto& renderableSprite = 
			entity.GetComponent<SpriteRenderableComponent>().sprite;

		CHECK(spriteAtlas.IsSpriteValid(renderableSprite));
		CHECK(renderableSprite == sprites[0]);

		CHECK_FALSE(entity.HasComponent<NeedsAnimationUpdate>());

		// mutate sprite animation component to get updated
		REQUIRE(entity.HasComponent<SpriteAnimationComponent>());
		auto& animComponent = entity.GetComponent<SpriteAnimationComponent>();
		CHECK(entity.HasComponent<NeedsAnimationUpdate>());

		CHECK(animComponent.index.current == 0);
		++animComponent.index;

		fixture->StepGameLoop(1);
		// max index for this series should have been updated
		CHECK(animComponent.index.max == 8);

		REQUIRE(entity.HasComponent<SpriteRenderableComponent>());
		const auto& nextRenderableSprite =
			entity.GetComponent<SpriteRenderableComponent>().sprite;

		CHECK(spriteAtlas.IsSpriteValid(nextRenderableSprite));
		CHECK(nextRenderableSprite == sprites[1]);

		CHECK_FALSE(entity.HasComponent<NeedsAnimationUpdate>());
	}

	SECTION("Out of bounds index on SpriteAnimationComponent defaults to 0")
	{
		REQUIRE(entity.HasComponent<SpriteRenderableComponent>());
		CHECK_FALSE(entity.GetComponent<SpriteRenderableComponent>()
			.sprite.resourceHandle.IsValid());

		entity.AddComponent(SpriteAnimationComponent{
			.spriteSeriesName = "sword_slash",
			.index = { .current = 999 }
		});

		CHECK(entity.HasComponent<NeedsAnimationUpdate>());

		fixture->StepGameLoop(1);

		REQUIRE(entity.HasComponent<SpriteRenderableComponent>());
		const auto& renderableSprite =
			entity.GetComponent<SpriteRenderableComponent>().sprite;

		CHECK(spriteAtlas.IsSpriteValid(renderableSprite));
		CHECK(renderableSprite == sprites[0]);

		CHECK_FALSE(entity.HasComponent<NeedsAnimationUpdate>());

		REQUIRE(entity.HasComponent<SpriteAnimationComponent>());
		CHECK(entity.GetComponent<SpriteAnimationComponent>().index.current == 0);
		CHECK(entity.GetComponent<SpriteAnimationComponent>().index.max == 8);
	}
}