#include "NewRenderSystem.h"
#include <SDL.h>
#include <ranges>
#include <numbers>
#include "../ecs/Ecs.h"
#include "../camera/Camera.h"
#include "../atlas/NewTextureRepository.h"
#include "util/RenderUtils.h"

namespace {

constexpr SDL_FRect ApplyOffset(SDL_FRect rect, SDL_FPoint offset)
{
	return SDL_FRect{
		.x = rect.x + offset.x,
		.y = rect.y + offset.y,
		.w = rect.w,
		.h = rect.h
	};
}

static SDL_FPoint RotatePointAround(const SDL_FPoint& p, const SDL_FPoint& center, float angleDeg)
{
	if (angleDeg == 0.0f) { return p; }

	const float rad = angleDeg * (std::numbers::pi_v<float> / 180.0f);
	const float c = std::cos(rad);
	const float s = std::sin(rad);
	const float dx = p.x - center.x;
	const float dy = p.y - center.y;

	return SDL_FPoint{ 
		center.x + (dx * c - dy * s), 
		center.y + (dx * s + dy * c) 
	};
}

static SDL_FRect RotateRectCenterAround(const SDL_FRect& rect, const SDL_FPoint& pivot, float angleDeg)
{
	if (angleDeg == 0.0f) { return rect; }

	const SDL_FPoint rectCenter{ 
		rect.x + rect.w * 0.5f, 
		rect.y + rect.h * 0.5f 
	};

	const SDL_FPoint newCenter = RotatePointAround(rectCenter, pivot, angleDeg);
	
	return SDL_FRect{ 
		newCenter.x - rect.w * 0.5f, 
		newCenter.y - rect.h * 0.5f, 
		rect.w, 
		rect.h 
	};
}

constexpr SDL_FRect ScaleRectAboutCenter(const SDL_FRect& rect, float scale)
{
	SDL_FPoint rectCenter{
		rect.x + rect.w * 0.5f,
		rect.y + rect.h * 0.5f
	};

	float scaledW = rect.w * scale;
	float scaledH = rect.h * scale;

	return SDL_FRect{
		rectCenter.x - (scaledW / 2.0f),
		rectCenter.y - (scaledH / 2.0f),
		scaledW,
		scaledH
	};
}
constexpr SDL_FRect MakeTransformedRect(const Transform& transform, int w, int h,
										const RenderProfile& profile)
{
	float scaledW = static_cast<float>(w) * transform.scale.x;
	float scaledH = static_cast<float>(h) * transform.scale.y;

	return SDL_FRect{
		transform.position.x - (scaledW / 2.0f) + profile.offset.x,
		transform.position.y - (scaledH / 2.0f) + profile.offset.y,
		scaledW,
		scaledH	
	};
}

SDL_FRect MakeScreenRenderRect(const Camera& camera, const Transform& transform, 
							   int w, int h, const RenderProfile& profile)
{
	const float zoomScale = (!profile.isOverlay) ? camera.GetZoomScale() : 1.0f;

	float scaledW = static_cast<float>(w) * transform.scale.x * zoomScale;
	float scaledH = static_cast<float>(h) * transform.scale.y * zoomScale;

	SDL_FPoint resolvedXY = transform.position + profile.offset;

	if (!profile.isOverlay)
	{
		resolvedXY = camera.WorldToScreen<SDL_FPoint>(resolvedXY);
	}
	else if (profile.parallaxFactor != 0.0f)
	{
		SDL_FPoint screenXY = camera.WorldToScreen<SDL_FPoint>(resolvedXY);
		resolvedXY = screenXY + (resolvedXY - screenXY) * profile.parallaxFactor;
	}

	return SDL_FRect{
		resolvedXY.x - (scaledW / 2.0f),
		resolvedXY.y - (scaledH / 2.0f),
		scaledW,
		scaledH	
	};
}

SDL_FRect RotatedRectToAABB(const SDL_FRect& rect, float rotationDegrees)
{
	if (rotationDegrees == 0.0f)
	{
		return rect;
	}

	const float cx = rect.x + rect.w * 0.5f;
	const float cy = rect.y + rect.h * 0.5f;
	const float hw = rect.w * 0.5f;
	const float hh = rect.h * 0.5f;

	const float rad = rotationDegrees * (std::numbers::pi_v<float> / 180.0f);
	const float c = std::cos(rad);
	const float s = std::sin(rad);

	std::array<SDL_FPoint, 4> localCorners = {
		SDL_FPoint{ -hw, -hh }, // top-left
		SDL_FPoint{  hw, -hh }, // top-right
		SDL_FPoint{  hw,  hh }, // bottom-right
		SDL_FPoint{ -hw,  hh }  // bottom-left
	};

	float minX = std::numeric_limits<float>::infinity();
	float minY = std::numeric_limits<float>::infinity();
	float maxX = -std::numeric_limits<float>::infinity();
	float maxY = -std::numeric_limits<float>::infinity();

	for (const auto& lc : localCorners)
	{
		const float rx = cx + (lc.x * c - lc.y * s);
		const float ry = cy + (lc.x * s + lc.y * c);

		if (rx < minX) { minX = rx; }
		if (ry < minY) { minY = ry; }
		if (rx > maxX) { maxX = rx; }
		if (ry > maxY) { maxY = ry; }
	}

	return SDL_FRect{ minX, minY, maxX - minX, maxY - minY };
}


bool ScreenRectIntersectsViewport(const Camera& camera, SDL_FRect testRect,
								  float rotation)
{
	if (camera.GetZoomScale() != 1.0f)
	{
		testRect = ScaleRectAboutCenter(testRect, camera.GetZoomScale());
	}
	if (rotation != 0.0f)
	{
		testRect = RotatedRectToAABB(testRect, rotation);
	}

	return camera.GetViewport().IntersectsBoundingBox(testRect);
}

SDL_FPoint GetScreenAdjust(const Camera& camera, SDL_FRect rect)
{
	SDL_FPoint rectXY = { rect.x, rect.y };
	SDL_FPoint screenXY = camera.WorldToScreen<SDL_FPoint>(rectXY);

	return screenXY - rectXY;
}

template <typename T>
constexpr T BitIf(bool condition, T flag) noexcept
{
	return static_cast<T>(-static_cast<int>(condition) & static_cast<int>(flag));
}

const Handle<TextureResource>& GetTextureResourceHandle(const Entity& e)
{
	return (e.HasComponent<SpriteRenderableComponent>())
		? e.GetComponent<SpriteRenderableComponent>().sprite.resourceHandle
		: e.GetComponent<TextRenderableComponent>().writer.resourceHandle;
}

enum : uint8_t 
{
	LhsValid = 1 << 0,
	RhsValid = 1 << 1,
	HandlesEq = 1 << 2,
	LhsLess = 1 << 3
};

uint8_t EvaluateTextureResourceHandles(const Entity& lhs, const Entity& rhs)
{
	const auto& lhsHandle = GetTextureResourceHandle(lhs);
	const auto& rhsHandle = GetTextureResourceHandle(rhs);

	return (
		BitIf(lhsHandle.IsValid(), LhsValid)     |
		BitIf(rhsHandle.IsValid(), RhsValid)     |
		BitIf(lhsHandle.GetAtlasID() == rhsHandle.GetAtlasID(), HandlesEq) |
		BitIf(lhsHandle.GetAtlasID() < rhsHandle.GetAtlasID(), LhsLess)
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
		const uint8_t handleEval = EvaluateTextureResourceHandles(lhs, rhs);
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

bool SameAtlasID(const Entity& lhs, const Entity& rhs)
{
	return GetTextureResourceHandle(lhs).GetAtlasID() == 
		   GetTextureResourceHandle(rhs).GetAtlasID();
}

auto ChunkEntitiesByAtlas(const std::vector<Entity>& entities)
{
	return entities 
		| std::views::chunk_by(SameAtlasID)
		| std::views::drop_while([](auto group) {
			return !GetTextureResourceHandle(group.front()).IsValid();
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
					).sprite.resourceHandle.IsValid()
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
	auto [renderable, transform] =
		entity.GetComponents<SpriteRenderableComponent, Transform>();

	const auto& plot = renderable.sprite.plot;

	SDL_FRect transformedRect = MakeTransformedRect(transform, plot.rect.w,
													plot.rect.h, renderable.profile);

	float displayRotation = transform.rotation + plot.rotation;

	if (!renderable.profile.isOverlay)
	{
		if (!ScreenRectIntersectsViewport(camera, transformedRect, displayRotation))
		{
			return;
		}

		displayRotation += camera.GetRotation();
		transformedRect = camera.WorldToScreen<SDL_FRect>(transformedRect);
	}

	renderBatchHandler.PushBack({
		.srcRect = plot.rect,
		.destRect = transformedRect,
		.rotationAngle = static_cast<double>(displayRotation),
		.mods = renderable.profile.mods,
		.flip = renderable.profile.flip
	});

	const auto& [debugBbox, debugCollider] = renderable.profile.debugDraw;
	if (debugBbox.on)
	{
		debugDrawHandler.AddBoundingBox(transformedRect, displayRotation,
										renderable.profile);
	}
	if (debugCollider.on && entity.HasComponent<Collider>())
	{
		debugDrawHandler.AddColliderShape(camera, entity.GetComponent<Collider>(),
										  renderable.profile);
	}
}

//// TODO: Fix rotation calculation for camera
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
		if (glyph == FontAtlasTexture::kNewlineGlyph)
		{
			continue;
		}

		// was pre-transformed by glyph caching in preprocessor, but need to apply offset from profile
		SDL_FRect transformedRect = ApplyOffset(destRect, renderable.profile.offset);

		float displayRotation = transform.rotation + glyph.plot.rotation;
		SDL_FPoint displayRotationCenter = rotationCenter + renderable.profile.offset;

		if (!renderable.profile.isOverlay)
		{
			if (!ScreenRectIntersectsViewport(camera, transformedRect, displayRotation))
			{
				continue;
			}

			displayRotation += camera.GetRotation();
			transformedRect = camera.WorldToScreen<SDL_FRect>(transformedRect);
		}

		renderBatchHandler.PushBack({
			.srcRect = glyph.plot.rect,
			.destRect = transformedRect,
			.rotationAngle = displayRotation,
			.rotationCenter = displayRotationCenter,
			.mods = renderable.profile.mods,
			.flip = renderable.profile.flip
		});
		 
		const auto& [debugBbox, debugCollider] = renderable.profile.debugDraw;
		if (debugBbox.on)
		{
			debugDrawHandler.AddBoundingBox(transformedRect, displayRotation,
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

void DrawDebugColliderShapeForInvalids(auto& group, const Camera& camera, 
									   DebugDrawHandler& debugDrawHandler)
{
	for (const auto& entity : group)
	{
		if (!entity.HasComponent<Collider>())
		{
			continue;
		}

		const auto& profile = entity.HasComponent<SpriteRenderableComponent>()
			? entity.GetComponent<SpriteRenderableComponent>().profile
			: entity.GetComponent<TextRenderableComponent>().profile;

		if (profile.debugDraw.collider.on)
		{
			debugDrawHandler.AddColliderShape(camera, entity.GetComponent<Collider>(),
											  profile);
		}
	}
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

	auto chunked = entities | std::views::chunk_by(SameAtlasID);

	for (auto group : chunked)
	{
		const auto& resourceHandle = GetTextureResourceHandle(group.front());

		if (!resourceHandle.IsValid())
		{
			// still draw debug collider if relevant
			DrawDebugColliderShapeForInvalids(group, camera, debugDrawHandler_);
			continue;
		}

		auto* srcTexture = textureRepo.GetSourceTexture(resourceHandle);
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