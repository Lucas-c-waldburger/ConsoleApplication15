#include "SpriteAnimationSystem.h"
#include "../ecs/Ecs.h"
#include "../atlas/TextureRepository.h"

void SpriteAnimationSystem::Update(const TextureRepository& textureRepo)
{
	auto entities = ECS::GetAllEntitiesWith<NeedsUpdate, Renderable, SpriteAnimations>();

	for (auto& entity : entities)
	{
		auto& update = entity.GetComponent<NeedsUpdate>();

		if ((update.components & SpriteAnimations::componentBit) == 0)
		{
			continue;
		}

		update.components &= ~(SpriteAnimations::componentBit);
		if (update.components == 0)
		{
			entity.RemoveComponent<NeedsUpdate>();
		}

		auto& renderable = entity.GetComponent<Renderable>();
		auto* spriteRenderable = std::get_if<SpriteRenderable>(&renderable.renderData);

		if (!spriteRenderable)
		{
			continue;
		}

		auto& animations = entity.GetComponent<SpriteAnimations>();

		Handle<SpriteSeriesAtlas> newAtlas{};
		AtlasPlot newPlot{};

		auto it = animations.table.find(animations.current);
		if (it == animations.table.end())
		{
			renderable.renderData = std::monostate{};
			continue;
		}

		const auto& series = it->second;

		assert(series.index < series.spritePlots.size());

		newAtlas = series.sourceAtlas;
		newPlot = series.spritePlots[series.index];
		LOG_DEBUG_FMT("series index: {}", series.index);
			
		spriteRenderable->sourceAtlas = newAtlas;
		spriteRenderable->sourcePlot = newPlot;
	}
}
