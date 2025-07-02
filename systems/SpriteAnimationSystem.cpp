#include "SpriteAnimationSystem.h"
#include "../ecs/Ecs.h"
#include "../atlas/TextureRepository.h"

namespace {

bool NeedsAnimationHandling(const SpriteAnimations& animations, const NewRenderable& renderable)
{
	return animations.map.NeedsUpdate() && animations.map.HasCurrent() && 
		   std::holds_alternative<SpriteRenderable>(renderable.renderData);
}

} // unnamed

void SpriteAnimationSystem::Update(const TextureRepository& textureRepo)
{
	// TODO: Figure out if we can just feed &NeedsHandling directly as filter arg
	auto entities = ECS::GetAllEntitiesWith<SpriteAnimations, NewRenderable>(
		[](const SpriteAnimations& animations, const NewRenderable& renderable) {
			return NeedsAnimationHandling(animations, renderable);
		});

	for (auto& entity : entities)
	{
		auto& animations = entity.GetComponent<SpriteAnimations>().map;

		const auto* currentSeries = animations.GetCurrent();
		assert(currentSeries);

		auto& renderable = entity.GetComponent<NewRenderable>();
		auto& spriteRenderable = std::get<SpriteRenderable>(renderable.renderData);

		assert(currentSeries->index < currentSeries->spritePlots.size());

		spriteRenderable.sourceAtlas = currentSeries->sourceAtlas;
		spriteRenderable.sourcePlot = currentSeries->spritePlots[currentSeries->index];

		animations.MarkUpdated();
	}
}
