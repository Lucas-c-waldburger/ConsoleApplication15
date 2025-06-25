#include "SpriteAnimationSystem.h"
#include "../ecs/Ecs.h"
#include "../atlas/TextureRepository.h"

void UpdateSpriteAnimations(const TextureRepository& textureRepo)
{
	auto entities = ECS::GetAllEntitiesWith<SpriteAnimations, NewRenderable>(
		[](const SpriteAnimations& spriteAnims, const NewRenderable&) {
			return spriteAnims.dirty;
		});

	for (auto& entity : entities)
	{
		auto& animations = entity.GetComponent<SpriteAnimations>();
		if (!animations.map.HasCurrent())
		{
			animations.dirty = false;
			continue;
		}

		auto& current = *animations.map.GetCurrent();
		if (current.spritePlots.empty())
		{
			LOG_WARNING("Sprite animation series's sprite plots was empty");

			animations.dirty = false;
			continue;
		}

		if (current.index >= current.spritePlots.size())
		{
			LOG_WARNING("Sprite animation series had an out-of-bounds index");

			current.index = current.spritePlots.size() - 1;
		}

		auto& renderable = entity.GetComponent<NewRenderable>();
		auto* spriteRenderable = std::get_if<SpriteRenderable>(&renderable.renderData);
		if (!spriteRenderable)
		{
			LOG_WARNING("Entity had sprite animation series component but no sprite renderable");

			animations.dirty = false;
			continue;
		}

		spriteRenderable->sourceAtlas = current.sourceAtlas;
		spriteRenderable->sourcePlot = current.spritePlots[current.index];

		animations.dirty = false;
	}
}
