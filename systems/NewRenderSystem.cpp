#include "NewRenderSystem.h"
#include <SDL.h>
#include <ranges>
#include "../ecs/Ecs.h"
#include "../camera/Camera.h"
#include "../atlas/NewTextureRepository.h"
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

template <typename T>
constexpr T BitIf(bool condition, T flag) noexcept
{
	return static_cast<T>(-static_cast<int>(condition) & static_cast<int>(flag));
}

const Handle<TextureAtlas>& GetAtlasHandle(const Entity& e)
{
	return (e.HasComponent<SpriteRenderableComponent>())
		? e.GetComponent<SpriteRenderableComponent>().sprite.sourceAtlas
		: e.GetComponent<TextRenderableComponent>().writer.sourceAtlas;
}

enum : uint8_t 
{
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

bool SameAtlasHandle(const Entity& lhs, const Entity& rhs)
{
	return GetAtlasHandle(lhs) == GetAtlasHandle(rhs);
}

auto ChunkEntitiesByAtlas(const std::vector<Entity>& entities)
{
	return entities 
		| std::views::chunk_by(SameAtlasHandle)
		| std::views::drop_while([](auto group) {
			return !GetAtlasHandle(group.front()).IsValid();
		});
}

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
		if (glyph == GlyphAtlas::kNewlineGlyph)
		{
			continue;
		}

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

void FillRenderBatches(const Entity& entity, const Camera& camera,
					   RenderBatchHandler& renderBatchHandler,
					   DebugDrawHandler& debugDrawHandler)
{
	(entity.HasComponent<SpriteRenderableComponent>())
		? AddSpriteRenderCall(entity, camera, renderBatchHandler, debugDrawHandler)
		: AddGlyphRenderCalls(entity, camera, renderBatchHandler, debugDrawHandler);
}

} // unnamed

void NewRenderSystem::Update(SDL_Renderer* renderer, const Camera& camera,
							 const TextureRepository& textureRepo)
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

	auto chunked = ChunkEntitiesByAtlas(entities);

	for (auto group : chunked)
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