#include "SpriteAnimationSystem.h"
#include "../ecs/Ecs.h"
#include "../atlas/NewTextureRepository.h"

void SpriteAnimationSystem::Update(const TextureRepository& textureRepo)
{
	if (IsPaused())
	{
		return;
	}

	auto entities = ECS::GetAllEntitiesWith<SpriteRenderableComponent, 
											SpriteAnimationComponent,
											NeedsAnimationUpdate>();

	for (auto& e : entities)
	{
		auto [renderable, anim] = e.GetComponents<SpriteRenderableComponent,
												  SpriteAnimationComponent>();

		e.RemoveComponent<NeedsAnimationUpdate>(GetEntityPassKey());

		const auto& spriteAtlas = textureRepo.GetSpriteAtlas();

		const size_t seriesSize = 
			spriteAtlas.GetSpriteSeriesSize(anim.spriteSeriesName);

		assert(seriesSize > 0);

		if (seriesSize == std::numeric_limits<size_t>::max())
		{
			LOG_ERROR_FMT("Sprite series '{}' not found in atlas", 
				anim.spriteSeriesName);
			continue;
		}

		anim.index.max = seriesSize - 1;

		if (anim.index.current > seriesSize)
		{
			LOG_ERROR_FMT("Sprite series '{}' max index '{}' exceeds current "
				"animation index. Defaulting to 0", 
				anim.spriteSeriesName, seriesSize);

			anim.index.current = 0;
		}

		auto newSprite = spriteAtlas.GetSpriteSeriesMember(anim.spriteSeriesName, 
														   anim.index.current);
		if (!newSprite.resourceHandle.IsValid())
		{
			LOG_ERROR("Could not retrieve new sprite for animation series");
			continue;
		}

		renderable.sprite = newSprite;
	}
}