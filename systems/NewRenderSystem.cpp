#include "NewRenderSystem.h"
#include <SDL.h>
#include <ranges>
#include "../ecs/Ecs.h"
#include "../camera/Camera.h"
#include "../atlas/TextureRepository.h"

namespace {

constexpr auto GetRenderableSourceAtlasHandle(const NewRenderable& renderable)
{
	return std::visit(Overloaded{
		[](const NewSpriteRenderable& sp) { return sp.sprite.sourceAtlas; },
		[](const NewTextRenderable& txt)  { return txt.writer.sourceAtlas; }
		}, renderable.renderData);
}
constexpr auto GetRenderableSourceAtlasHandles(const NewRenderable& lhs, 
											   const NewRenderable& rhs)
{
	return std::make_pair(GetRenderableSourceAtlasHandle(lhs),
						  GetRenderableSourceAtlasHandle(rhs));
}

constexpr bool CompRenderables(const NewRenderable& lhs, const NewRenderable& rhs)
{
	if (lhs.profile.drawOrder != rhs.profile.drawOrder)
	{
		return lhs.profile.drawOrder < rhs.profile.drawOrder;
	}

	auto [lhsSourceAtlas, rhsSourceAtlas] = GetRenderableSourceAtlasHandles(lhs, rhs);
	if (lhsSourceAtlas != rhsSourceAtlas)
	{
		return lhsSourceAtlas < rhsSourceAtlas;
	}

	return lhs.profile.mods.color != rhs.profile.mods.color 
		 ? lhs.profile.mods.color < rhs.profile.mods.color 
		 : lhs.profile.mods.alpha != rhs.profile.mods.alpha 
		 ? lhs.profile.mods.alpha < rhs.profile.mods.alpha 
		 : lhs.profile.mods.blend < rhs.profile.mods.blend;
} 

void SortRenderableEntities(std::vector<Entity>& entities)
{
	std::sort(entities.begin(), entities.end(), [](const Entity& lhs, const Entity& rhs) {
		return CompRenderables(lhs.GetComponent<NewRenderable>(), 
							   rhs.GetComponent<NewRenderable>());
	});
}

constexpr auto SameAtlasHandle(const Entity& lhs, const Entity& rhs)
{
	auto [lhsSourceAtlas, rhsSourceAtlas] = GetRenderableSourceAtlasHandles(
		lhs.GetComponent<NewRenderable>(), rhs.GetComponent<NewRenderable>()
	);

	return lhsSourceAtlas == rhsSourceAtlas;
}

size_t ComputeRenderCallCount(const std::vector<Entity>& entities)
{
	static constexpr auto getSpriteCount = [](const NewSpriteRenderable& sp) {
		return sp.sprite.sourceAtlas.IsValid() ? 1 : 0;
	};
	static constexpr auto getGlyphCount = [](const NewTextRenderable& txt) {
		return txt.writer.sourceAtlas.IsValid() ? txt.writer.text.size() : 0;
	};

	return std::accumulate(entities.begin(), entities.end(), 0,
		[](int sum, const Entity& e) {
			const auto& r = e.GetComponent<NewRenderable>();
			return sum + std::visit(Overloaded{
				getSpriteCount,
				getGlyphCount
			}, r.renderData);
		});
}

RenderCallArgs MakeSpriteRenderCallArgs(const NewSpriteRenderable& spriteRenderable, 
										const RenderProfile& profile,
										const Transform& transform, const Camera& camera)
{
	const auto& plot = spriteRenderable.sprite.plot;

	SDL_Rect renderRect = MakeScreenRect(camera, transform, plot.rect.w, 
										 plot.rect.h, profile.offset);

	if (!camera.GetViewport().IntersectsBoundingBox(renderRect))
	{
		return;
	}
	 
	return RenderCallArgs{
		.srcRect = plot.rect,
		.destRect = renderRect,
		.rotationAngle = static_cast<double>(transform.rotation + plot.rotation),
		.mods = profile.mods,
		.flip = profile.flip
	};
}



void FillRenderBatch(const Entity& entity, RenderBatchHandler& renderBatchHandler, 
					 const Camera& camera)
{

}

} // unnamed

void NewRenderSystem::Update(SDL_Renderer* renderer, const Camera& camera, 
	                         const NewTextureRepository& textureRepo)
{
	auto entities = ECS::GetAllEntitiesWith<Transform, NewRenderable>();

	renderBatchHandler_.Clear();
	renderBatchHandler_.Reserve(ComputeRenderCallCount(entities));

	SortRenderableEntities(entities);

	SDL_Color originalDrawColor = GetRenderDrawColor(renderer);

	for (auto group : entities | std::views::chunk_by(SameAtlasHandle))
	{
		auto srcAtlasHandle = GetRenderableSourceAtlasHandle(
			group.front().GetComponent<NewRenderable>());

		auto* srcTexture = textureRepo.GetSourceTexture(srcAtlasHandle);
		if (!srcTexture)
		{
			LOG_ERROR("Atlas handle was invalid or expired");
			continue;
		}

		renderBatchHandler_.StartRenderBatch(srcTexture);

		for (const auto& entity : group)
		{
			FillRenderBatch(entity, renderBatchHandler_);
		}
	}

}
