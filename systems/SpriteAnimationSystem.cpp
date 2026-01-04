#include "SpriteAnimationSystem.h"
#include "../ecs/Ecs.h"
#include "../atlas/NewTextureRepository.h"

void SpriteAnimationSystem::Update(const TextureRepository& textureRepo)
{
	auto entities = ECS::GetAllEntitiesWith<SpriteRenderableComponent, 
											SpriteAnimationComponent,
											NeedsAnimationUpdate>();

	for (auto& e : entities)
	{
		auto [renderable, anim] = e.GetComponents<SpriteRenderableComponent,
												  SpriteAnimationComponent>();

		e.RemoveComponent<NeedsAnimationUpdate>(GetEntityPassKey());

		auto* atlas = textureRepo.GetAtlas<SpriteAtlas>(anim.sourceAtlas);
		if (!atlas)
		{ 
			continue;
		}

		const size_t seriesSize = atlas->GetSpriteSeriesSize(anim.spriteSeriesName);
		assert(seriesSize > 0);

		if (seriesSize == std::numeric_limits<size_t>::max())
		{
			LOG_ERROR_FMT("Sprite series '{}' not found in atlas", 
				anim.spriteSeriesName);
			continue;
		}

		if (anim.currentIndex >= seriesSize)
		{
			LOG_ERROR_FMT("Sprite series '{}' size '{}' exceeds current animation "
				"index. Defaulting to 0", anim.spriteSeriesName, seriesSize);

			anim.currentIndex = 0;
		}

		auto newSprite = atlas->GetSpriteSeriesMember(anim.spriteSeriesName, 
													  anim.currentIndex);
		if (!atlas->IsSpriteValid(newSprite))
		{
			LOG_ERROR("Could not retrieve new sprite for animation series");
			continue;
		}

		renderable.sprite = newSprite;
	}
}