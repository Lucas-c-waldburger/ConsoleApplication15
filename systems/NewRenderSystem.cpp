#include "NewRenderSystem.h"
#include <SDL.h>
#include <ranges>
#include "../ecs/Ecs.h"
#include "../camera/Camera.h"
#include "../atlas/NewTextureRepository.h"
#include "RenderablePreProcessorSystem.h"
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

//constexpr auto GetRenderableSourceAtlasHandle(const NewRenderable& renderable)
//{
//	return std::visit(Overloaded{
//		[](const NewSpriteRenderable& sp) { return sp.sprite.sourceAtlas; },
//		[](const NewTextRenderable& txt)  { return txt.writer.sourceAtlas; }
//		}, renderable.renderData);
//}
//
//void UpdateEntityRenderableInternals(std::vector<Entity>& entities)
//{
//	for (auto& entity : entities)
//	{
//		auto& renderable = entity.GetComponent<NewRenderable>();
//		renderable.internals_.sourceAtlas = GetRenderableSourceAtlasHandle(renderable);
//	}
//}

template <typename T>
constexpr T BitIf(bool condition, T flag) noexcept
{
	return static_cast<T>(-static_cast<int>(condition) & static_cast<int>(flag));
}

//bool RenderableValid(const NewRenderable& r)
//{
//	return r.internals_.sourceAtlas.IsValid() && r.internals_.renderCallCount > 0;
//}

const Handle<NewTextureAtlas>& GetAtlasHandle(const Entity& e)
{
	return (e.HasComponent<SpriteRenderableComponent>())
		? e.GetComponent<SpriteRenderableComponent>().sprite.sourceAtlas
		: e.GetComponent<TextRenderableComponent>().writer.sourceAtlas;
}

enum : uint8_t {
	LhsValid = 1 << 0,
	RhsValid = 1 << 1,
	HandlesEq = 1 << 2,
	LhsLess = 1 << 3
};

uint8_t EvaluateAtlasHandles(const Entity& lhs, const Entity& rhs)
{
	const auto& lhsHandle = GetAtlasHandle(lhs);
	const auto& rhsHandle = GetAtlasHandle(rhs);

	return (
		BitIf(lhsHandle.IsValid(), LhsValid)     |
		BitIf(rhsHandle.IsValid(), RhsValid)     |
		BitIf(lhsHandle == rhsHandle, HandlesEq) |
		BitIf(lhsHandle < rhsHandle, LhsLess)
	);
}

bool CompRenderables(const RenderProfile& lhs, const RenderProfile& rhs, uint8_t handleEval)
{
	return lhs.drawOrder != rhs.drawOrder
		 ? lhs.drawOrder < rhs.drawOrder
		 : ((handleEval & HandlesEq) == 0) 
		 ? static_cast<bool>(handleEval & LhsLess)
		 : lhs.mods.color != rhs.mods.color
		 ? lhs.mods.color < rhs.mods.color
		 : lhs.mods.alpha != rhs.mods.alpha
		 ? lhs.mods.alpha < rhs.mods.alpha
		 : lhs.mods.blend < rhs.mods.blend;
}

const RenderProfile& GetRenderProfile(const Entity& e)
{
	return (e.HasComponent<SpriteRenderableComponent>())
		? e.GetComponent<SpriteRenderableComponent>().profile
		: e.GetComponent<TextRenderableComponent>().profile;
}

void SortRenderableEntities(std::vector<Entity>& entities)
{
	std::sort(entities.begin(), entities.end(), [](const Entity& lhs, const Entity& rhs)
	{
		const uint8_t handleEval = EvaluateAtlasHandles(lhs, rhs);
		const uint8_t validTest = handleEval & (LhsValid | RhsValid);

		// If lhs invalid Å® goes before rhs if rhs valid
		if ((validTest & LhsValid) == 0)
		{
			return static_cast<bool>(validTest & RhsValid);
		}

		// If both invalid Å® keep order, if both valid Å® compare normally
		if ((validTest & RhsValid) == 0)
		{
			return false;
		}

		return CompRenderables(GetRenderProfile(lhs), GetRenderProfile(rhs), handleEval);
	});
}


//bool CompRenderables(const NewRenderable& lhs, const NewRenderable& rhs)
//{
//	bool lhsValid = RenderableValid(lhs);
//	bool rhsValid = RenderableValid(rhs);
//
//	if (!lhsValid && rhsValid)  return true;   // lhs invalid Å® front
//	if (lhsValid && !rhsValid)  return false;  // rhs invalid Å® front
//	if (!lhsValid && !rhsValid) return false; // keep relative order 
//
//	// both valid Å® normal sorting
//	return lhs.profile.drawOrder != rhs.profile.drawOrder
//		? lhs.profile.drawOrder < rhs.profile.drawOrder
//		: lhs.internals_.sourceAtlas != rhs.internals_.sourceAtlas
//		? lhs.internals_.sourceAtlas < rhs.internals_.sourceAtlas
//		: lhs.profile.mods.color != rhs.profile.mods.color
//		? lhs.profile.mods.color < rhs.profile.mods.color
//		: lhs.profile.mods.alpha != rhs.profile.mods.alpha
//		? lhs.profile.mods.alpha < rhs.profile.mods.alpha
//		: lhs.profile.mods.blend < rhs.profile.mods.blend;
//}


//void SortRenderableEntities(std::vector<Entity>& entities)
//{
//	std::sort(entities.begin(), entities.end(), [](const Entity& lhs, const Entity& rhs) {
//		return CompRenderables(lhs.GetComponent<NewRenderable>(), 
//							   rhs.GetComponent<NewRenderable>());
//	});
//}



//constexpr auto SameAtlasHandle(const Entity& lhs, const Entity& rhs)
//{
//	return lhs.GetComponent<NewRenderable>().internals_.sourceAtlas ==
//		   rhs.GetComponent<NewRenderable>().internals_.sourceAtlas;
//}

bool SameAtlasHandle(const Entity& lhs, const Entity& rhs)
{
	return GetAtlasHandle(lhs) == GetAtlasHandle(rhs);
}

auto ChunkEntitiesByAtlas(const std::vector<Entity>& entities)
{
	auto chunkedByAtlas = entities | std::views::chunk_by(SameAtlasHandle);
	auto it = chunkedByAtlas.begin();
	if (it != chunkedByAtlas.end() && !GetAtlasHandle((*it).front()).IsValid())
	{
		++it;
	}

	return std::ranges::subrange(it, chunkedByAtlas.end());
}

//size_t ComputeRenderCallCount(const std::vector<Entity>& entities)
//{
//	return std::accumulate(entities.begin(), entities.end(), 0,
//		[](int sum, const Entity& e) {			
//			return sum + e.GetComponent<NewRenderable>().internals_.renderCallCount;
//		});
//}

size_t ComputeRenderCallCount(const std::vector<Entity>& entities)
{
	return std::accumulate(entities.begin(), entities.end(), 0,
		[](int sum, const Entity& e) 
		{
			return sum + 
			e.HasComponent<SpriteRenderableComponent>() 
				? static_cast<int>(e.GetComponent<SpriteRenderableComponent>(
					).sprite.sourceAtlas.IsValid()
				)
			: e.HasComponent<TextRenderableGlyphCache>() 
				? static_cast<int>(e.GetComponent<TextRenderableGlyphCache>().cache.size()) 
				: 0;
		});
}

//auto GetFirstValidIt(const std::vector<Entity>& entities)
//{
//	return std::find_if(entities.begin(), entities.end(), [](const Entity& e) {
//		return RenderableValid(e.GetComponent<NewRenderable>());
//	});
//}

void AddSpriteRenderCall(const Entity& entity, const Camera& camera,
						 RenderBatchHandler& renderBatchHandler,
						 DebugDrawHandler& debugDrawHandler)
{
	auto [renderable, transform] = entity.GetComponents<SpriteRenderableComponent, Transform>();

	const auto& plot = renderable.sprite.plot;

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
		entity.GetComponents<TextRenderableComponent, Transform, TextRenderableGlyphCache>();

	for (const auto& [glyph, destRect, rotationCenter] : glyphCache.cache)
	{
		// was pre-transformed by glyph caching in preprocessor
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

//void FillRenderBatches(const Entity& entity, const Camera& camera,
//					   RenderBatchHandler& renderBatchHandler, 
//					   DebugDrawHandler& debugDrawHandler)			   
//{
//	using E = const Entity&;
//	using C = const Camera&;
//	using RBH = RenderBatchHandler&;
//	using DDH = DebugDrawHandler&;
//	using FillBatchFunc = void(*)(E, C, RBH, DDH);
//
//	static constexpr auto dispatchTable = std::to_array<FillBatchFunc>({
//		[](E e, C c, RBH& rbh, DDH& ddh) { AddSpriteRenderCall(e, c, rbh, ddh); },
//		[](E e, C c, RBH& rbh, DDH& ddh) { AddGlyphRenderCalls(e, c, rbh, ddh); }
//	});
//
//	const auto& renderData = entity.GetComponent<NewRenderable>().renderData;
//	const size_t idx = renderData.index();
//	assert(idx < dispatchTable.size());
//
//	dispatchTable[idx](entity, camera, renderBatchHandler, debugDrawHandler);
//}

void FillRenderBatches(const Entity& entity, const Camera& camera,
					   RenderBatchHandler& renderBatchHandler,
					   DebugDrawHandler& debugDrawHandler)
{
	(entity.HasComponent<SpriteRenderableComponent>())
		? AddSpriteRenderCall(entity, camera, renderBatchHandler, debugDrawHandler)
		: AddGlyphRenderCalls(entity, camera, renderBatchHandler, debugDrawHandler);
}

} // unnamed

//void NewRenderSystem::Update(SDL_Renderer* renderer, const Camera& camera, 
//	                         const NewTextureRepository& textureRepo)
//{
//	auto entities = ECS::GetAllEntitiesWith<Transform, NewRenderable>();
//	if (entities.empty())
//	{
//		return;
//	}
//
//	renderablePreProcessor_.Update(textureRepo);
//
//	renderBatchHandler_.Clear();
//	renderBatchHandler_.Reserve(ComputeRenderCallCount(entities));
//
//	debugDrawHandler_.Clear();
//
//	SortRenderableEntities(entities);
//
//	// skip all invalid atlas handles moved to the front during sort
//	auto firstValidIt = GetFirstValidIt(entities);
//	if (firstValidIt == entities.end())
//	{
//		return;
//	}
//
//	auto validEntities = std::ranges::subrange(firstValidIt, entities.end());
//
//	for (auto group : validEntities | std::views::chunk_by(SameAtlasHandle))
//	{
//		const auto& srcAtlasHandle =
//			group.front().GetComponent<NewRenderable>().internals_.sourceAtlas;
//
//		auto* srcTexture = textureRepo.GetSourceTexture(srcAtlasHandle);
//		assert(srcTexture);
//		 
//		renderBatchHandler_.StartRenderBatch(srcTexture);
//
//		for (const auto& entity : group)
//		{
//			FillRenderBatches(entity, camera, renderBatchHandler_, debugDrawHandler_);
//		}
//	}
//
//	renderBatchHandler_.Render(renderer);
//	debugDrawHandler_.Draw(renderer);
//}

void NewRenderSystem::Update(SDL_Renderer* renderer, const Camera& camera,
							 const NewTextureRepository& textureRepo)
{
	auto entities = ECS::GetAllEntitiesWith<Transform, Any<SpriteRenderableComponent, 
														   TextRenderableComponent>>();
	if (entities.empty())
	{
		return;
	}

	renderablePreProcessor_.Update(textureRepo);

	renderBatchHandler_.Reset(renderablePreProcessor_.GetRenderCallCount());
	debugDrawHandler_.Reset(renderablePreProcessor_.GetDebugDrawPointCount());

	SortRenderableEntities(entities);

	for (auto group : ChunkEntitiesByAtlas(entities))
	{
		const auto& srcAtlasHandle = GetAtlasHandle(group.front());

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