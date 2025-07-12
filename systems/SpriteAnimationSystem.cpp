#include "SpriteAnimationSystem.h"
#include "../ecs/Ecs.h"
#include "../atlas/TextureRepository.h"

void SpriteAnimationSystem::Update(const TextureRepository& textureRepo)
{
	auto entities = ECS::GetAllEntitiesWith<NeedsUpdate, NewRenderable, SpriteAnimations>(
		[](const NeedsUpdate& update, const NewRenderable& renderable, const SpriteAnimations&)
		{
			return (update.components & SpriteAnimations::componentBit) &&
					std::holds_alternative<SpriteRenderable>(renderable.renderData);
		});

	for (auto& entity : entities)
	{
		auto& animations = entity.GetComponent<SpriteAnimations>();

		Handle<SpriteSeriesAtlas> newAtlas{};
		AtlasPlot newPlot{};

		auto it = animations.table.find(animations.current);
		if (it != animations.table.end())
		{
			const auto& series = it->second;

			assert(series.index < series.spritePlots.size());

			newAtlas = series.sourceAtlas;
			newPlot = series.spritePlots[series.index];
			LOG_DEBUG_FMT("series index: {}", series.index);
		}

		auto& renderable = entity.GetComponent<NewRenderable>();
		auto& spriteRenderable = std::get<SpriteRenderable>(renderable.renderData);

		spriteRenderable.sourceAtlas = newAtlas;
		spriteRenderable.sourcePlot = newPlot;

		auto& update = entity.GetComponent<NeedsUpdate>();
		update.components &= ~(SpriteAnimations::componentBit);
	}
}
