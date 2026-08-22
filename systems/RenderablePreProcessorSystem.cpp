#include "RenderablePreProcessorSystem.h"
#include "../ecs/Ecs.h"
#include "../atlas/NewTextureRepository.h"
#include "../render/GlyphCacheHandler.h"
#include "../physics/B2Shape.h"

namespace {

size_t EstimateDebugDrawPointCount(const Entity& entity, const RenderProfile& profile)
{
	size_t est = 0;
	if (profile.debugDraw.boundingBox.on)
	{
		est += 4;
	}
	if (profile.debugDraw.collider.on && entity.HasComponent<Collider>())
	{
		const auto& collider = entity.GetComponent<Collider>();
		const auto& shape = collider.shape.GetData();

		if (!shape.IsValid())
		{
			return est;
		}

		switch (shape.GetShapeType())
		{
		case B2Shape::Type::Polygon:
			est += shape.GetAs<B2PolygonShape>().GetVertexCount();
			break;
		case B2Shape::Type::Circle:
			est += std::max(static_cast<size_t>(shape.GetAs<B2CircleShape>().GetRadius()
						    * 2.0f * M_PI / 2.0f), 8_uz);
			break;
		default:
			break;
		}
	}

	return est;
}

} // unnamed

void RenderablePreProcessor::Update(const TextureRepository& textureRepo)
{
	auto entities = ECS::GetAllEntitiesWith<Transform, Any<SpriteRenderableComponent,
															TextRenderableComponent>>();
	Update(entities, textureRepo);
}

void RenderablePreProcessor::Update(std::vector<Entity>& entities, const TextureRepository& textureRepo)
{
	renderCallCount_ = 0;
	debugDrawPointCount_ = 0;

	for (auto& entity : entities)
	{
		const auto& transform = entity.GetComponent<Transform>();

		if (entity.HasComponent<SpriteRenderableComponent>())
		{
			auto& spriteRenderable = entity.GetComponent<SpriteRenderableComponent>();
			auto& [sprite, profile] = spriteRenderable;

			if (!textureRepo.GetSpriteAtlas().IsSpriteValid(sprite))
			{
				sprite.resourceHandle = {};
				continue;
			}

			++renderCallCount_;
			debugDrawPointCount_ += EstimateDebugDrawPointCount(entity, profile);
		}
		else
		{
			auto& textRenderable = entity.GetComponent<TextRenderableComponent>();
			auto& [writer, format, profile] = textRenderable;

			if (writer.text.empty())
			{
				ClearGlyphCache(entity);
				continue;
			}

			const auto& font = textureRepo.GetFontAtlas().GetFont(writer.resourceHandle);
			if (!font.IsLoaded())
			{
				writer.resourceHandle = {};
				ClearGlyphCache(entity);
				continue;
			}

			auto& glyphCache = 
				entity.AddComponent<TextRenderableGlyphCache>(GetEntityPassKey());

			GlyphCacheHandler::UpdateGlyphCache(font, textRenderable, 
												glyphCache, transform);

			renderCallCount_ += glyphCache.cache.size();
			debugDrawPointCount_ += EstimateDebugDrawPointCount(entity, profile);
		}
	}
}

void RenderablePreProcessor::ClearGlyphCache(Entity& entity)
{
	if (!entity.HasComponent<TextRenderableGlyphCache>())
	{
		return;
	}

	entity.GetComponent<TextRenderableGlyphCache>(GetEntityPassKey()) = 
		TextRenderableGlyphCache{};
}
