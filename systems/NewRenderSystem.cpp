#include "NewRenderSystem.h"
#include <SDL.h>
#include <ranges>
#include "../ecs/Ecs.h"
#include "../camera/Camera.h"
#include "../atlas/NewTextureRepository.h"
#include "GlyphFormattingSystem.h"
#include "util/GlyphFormattingUtils.h"
#include "util/RenderUtils.h"

namespace {

SDL_Rect MakeRenderDestRect(const Camera& camera, const Transform& transform,
							int w, int h, const RenderProfile& profile)
{
	float scaledW = w * transform.scale.x;
	float scaledH = h * transform.scale.y;

	SDL_FPoint renderXY = transform.position + profile.offset;

	if (!profile.isOverlay)
	{
		renderXY = camera.WorldToScreen<SDL_FPoint>(renderXY);
	}
	else if (profile.parallaxFactor != 0.0f)
	{
		SDL_FPoint screenXY = camera.WorldToScreen<SDL_FPoint>(renderXY);
		renderXY = screenXY + (renderXY - screenXY) * profile.parallaxFactor;
	}

	return SDL_Rect{
		static_cast<int>(renderXY.x - (scaledW / 2.0f)),
		static_cast<int>(renderXY.y - (scaledH / 2.0f)),
		static_cast<int>(scaledW),
		static_cast<int>(scaledH)
	};
}

SDL_Point GetScreenAdjust(const Camera& camera, SDL_Rect rect)
{
	SDL_Point rectXY = { rect.x, rect.y };
	SDL_Point screenXY = camera.WorldToScreen<SDL_Point>(rectXY);

	return screenXY - rectXY;
}

constexpr auto GetRenderableSourceAtlasHandle(const NewRenderable& renderable)
{
	return std::visit(Overloaded{
		[](const NewSpriteRenderable& sp) { return sp.sprite.sourceAtlas; },
		[](const NewTextRenderable& txt)  { return txt.writer.sourceAtlas; }
		}, renderable.renderData);
}

void UpdateEntityRenderableInternals(std::vector<Entity>& entities)
{
	for (auto& entity : entities)
	{
		auto& renderable = entity.GetComponent<NewRenderable>();
		renderable.internals_.sourceAtlas = GetRenderableSourceAtlasHandle(renderable);
	}
}

bool RenderableValid(const NewRenderable& r)
{
	return r.internals_.sourceAtlas.IsValid() && r.internals_.renderCallCount > 0;
}

bool CompRenderables(const NewRenderable& lhs, const NewRenderable& rhs)
{
	bool lhsValid = RenderableValid(lhs);
	bool rhsValid = RenderableValid(rhs);

	if (!lhsValid && rhsValid)  return true;   // lhs invalid Å® front
	if (lhsValid && !rhsValid)  return false;  // rhs invalid Å® front
	if (!lhsValid && !rhsValid) return false; // keep relative order 

	// both valid Å® normal sorting
	return lhs.profile.drawOrder != rhs.profile.drawOrder
		? lhs.profile.drawOrder < rhs.profile.drawOrder
		: lhs.internals_.sourceAtlas != rhs.internals_.sourceAtlas
		? lhs.internals_.sourceAtlas < rhs.internals_.sourceAtlas
		: lhs.profile.mods.color != rhs.profile.mods.color
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
	return lhs.GetComponent<NewRenderable>().internals_.sourceAtlas ==
		   rhs.GetComponent<NewRenderable>().internals_.sourceAtlas;
}

size_t ComputeRenderCallCount(const std::vector<Entity>& entities)
{
	return std::accumulate(entities.begin(), entities.end(), 0,
		[](int sum, const Entity& e) {			
			return sum + e.GetComponent<NewRenderable>().internals_.renderCallCount;
		});
}

auto GetFirstValidIt(const std::vector<Entity>& entities)
{
	return std::find_if(entities.begin(), entities.end(), [](const Entity& e) {
		return RenderableValid(e.GetComponent<NewRenderable>());
	});
}

void AddSpriteRenderCall(const Entity& entity, const Camera& camera,
						 RenderBatchHandler& renderBatchHandler,
						 DebugDrawHandler& debugDrawHandler)
{
	auto [renderable, transform] = entity.GetComponents<NewRenderable, Transform>();
	auto& spriteRenderable = std::get<NewSpriteRenderable>(renderable.renderData);

	const auto& plot = spriteRenderable.sprite.plot;

	SDL_Rect renderDestRect = MakeRenderDestRect(camera, transform, plot.rect.w, 
										         plot.rect.h, renderable.profile);

	if (!camera.GetViewport().IntersectsBoundingBox(renderDestRect))
	{
		return;
	}
	 
	renderBatchHandler.PushBack({
		.srcRect = plot.rect,
		.destRect = renderDestRect,
		.rotationAngle = static_cast<double>(transform.rotation + plot.rotation),
		.mods = renderable.profile.mods,
		.flip = renderable.profile.flip
	});

	const auto& [debugBbox, debugCollider] = renderable.profile.debugDraw;
	if (debugBbox.on)
	{
		debugDrawHandler.AddBoundingBox(renderDestRect, transform.rotation, 
										renderable.profile);
	}
	if (debugCollider.on && entity.HasComponent<Collider>())
	{
		debugDrawHandler.AddColliderShape(camera, entity.GetComponent<Collider>(), 
										  renderable.profile);
	}
}

void AddGlyphRenderCalls(const Entity& entity, const Camera& camera,
						 RenderBatchHandler& renderBatchHandler,
						 DebugDrawHandler& debugDrawHandler)
{
	if (!entity.HasComponent<TextRenderableGlyphCache>())
	{
		return;
	}

	const auto [renderable, transform, glyphCache] =
		entity.GetComponents<NewRenderable, Transform, TextRenderableGlyphCache>();

	for (const auto& [glyph, destRect, rotationCenter] : glyphCache.cache)
	{
		// was pre-transformed by glyph caching system
		SDL_Rect renderDestRect = destRect;
		SDL_Point renderRotationCenter = rotationCenter;
		
		if (!renderable.profile.isOverlay)
		{
			SDL_Point screenAdjust = GetScreenAdjust(camera, renderDestRect);

			renderDestRect.x += screenAdjust.x;
			renderDestRect.y += screenAdjust.y;

			renderRotationCenter += screenAdjust;
		}

		if (!camera.GetViewport().IntersectsBoundingBox(renderDestRect))
		{
			continue;
		}

		renderBatchHandler.PushBack({
			.srcRect = glyph.plot.rect,
			.destRect = renderDestRect,
			.rotationAngle = static_cast<double>(transform.rotation + glyph.plot.rotation),
			.rotationCenter = renderRotationCenter,
			.mods = renderable.profile.mods,
			.flip = renderable.profile.flip
		});

		const auto& [debugBbox, debugCollider] = renderable.profile.debugDraw;
		if (debugBbox.on)
		{
			debugDrawHandler.AddBoundingBox(renderDestRect, transform.rotation, 
											renderable.profile);
		}
		if (debugCollider.on && entity.HasComponent<Collider>())
		{
			debugDrawHandler.AddColliderShape(camera, entity.GetComponent<Collider>(),
				renderable.profile);
		}
	}
}

void FillRenderBatches(const Entity& entity, const Camera& camera,
					   RenderBatchHandler& renderBatchHandler, 
					   DebugDrawHandler& debugDrawHandler)			   
{
	using E = const Entity&;
	using C = const Camera&;
	using RBH = RenderBatchHandler&;
	using DDH = DebugDrawHandler&;
	using FillBatchFunc = void(*)(E, C, RBH, DDH);

	static constexpr auto dispatchTable = std::to_array<FillBatchFunc>({
		[](E e, C c, RBH& rbh, DDH& ddh) { AddSpriteRenderCall(e, c, rbh, ddh); },
		[](E e, C c, RBH& rbh, DDH& ddh) { AddGlyphRenderCalls(e, c, rbh, ddh); }
	});

	const auto& renderData = entity.GetComponent<NewRenderable>().renderData;
	const size_t idx = renderData.index();
	assert(idx < dispatchTable.size());

	dispatchTable[idx](entity, camera, renderBatchHandler, debugDrawHandler);
}

} // unnamed

void NewRenderSystem::Update(SDL_Renderer* renderer, const Camera& camera, 
	                         const NewTextureRepository& textureRepo)
{
	auto entities = ECS::GetAllEntitiesWith<Transform, NewRenderable>();
	if (entities.empty())
	{
		return;
	}

	renderablePreProcessor_.Update(textureRepo);

	renderBatchHandler_.Clear();
	renderBatchHandler_.Reserve(ComputeRenderCallCount(entities));

	debugDrawHandler_.Clear();

	SortRenderableEntities(entities);

	// skip all invalid atlas handles moved to the front during sort
	auto firstValidIt = GetFirstValidIt(entities);
	if (firstValidIt == entities.end())
	{
		return;
	}

	auto validEntities = std::ranges::subrange(firstValidIt, entities.end());

	for (auto group : validEntities | std::views::chunk_by(SameAtlasHandle))
	{
		const auto& srcAtlasHandle =
			group.front().GetComponent<NewRenderable>().internals_.sourceAtlas;

		auto* srcTexture = textureRepo.GetSourceTexture(srcAtlasHandle);
		assert(srcTexture);
		 
		renderBatchHandler_.StartRenderBatch(srcTexture);

		for (const auto& entity : group)
		{
			FillRenderBatches(entity, camera, renderBatchHandler_, debugDrawHandler_);
		}
	}

	renderBatchHandler_.Render(renderer);
	debugDrawHandler_.Draw(renderer);
}
