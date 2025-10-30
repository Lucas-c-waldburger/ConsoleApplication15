#include "RenderUtils.h"
#include "../../camera/Camera.h"
#include "../../components/TransformComponent.h"
#include <SDL_render.h>

namespace {

constexpr const SDL_Point* GetRotationCenter(const RenderCallArgs& args)
{
	return args.rotationCenter.has_value() ? &(*args.rotationCenter) : nullptr;
}

} // unnamed

void TextureModTracker::Update(SDL_Texture* texture, const TextureMods& newMods)
{
	if (newMods.color != trackedMods.color)
	{
		SDL_SetTextureColorMod(texture,
			ClampToLimits<uint8_t>(newMods.color.r),
			ClampToLimits<uint8_t>(newMods.color.g),
			ClampToLimits<uint8_t>(newMods.color.b));

		trackedMods.color = newMods.color;
	}
	if (newMods.alpha != trackedMods.alpha)
	{
		SDL_SetTextureAlphaMod(texture, ClampToLimits<uint8_t>(newMods.alpha));

		trackedMods.alpha = newMods.alpha;
	}
	if (newMods.blend != trackedMods.blend)
	{
		SDL_SetTextureBlendMode(texture, newMods.blend);

		trackedMods.blend = newMods.blend;
	}
}

void TextureModTracker::Reset(SDL_Texture* texture)
{
	Update(texture, {});
}

void RenderBatchHandler::StartRenderBatch(SDL_Texture* texture)
{
	if (!batches_.empty())
	{
		// Close the previous batch
		batches_.back().batchEndIndex = callArgs_.size();
	}

	batches_.emplace_back(texture, 0);
}

void RenderBatchHandler::Clear()
{
	batches_.clear();
	callArgs_.clear();
}

void RenderBatchHandler::Reserve(size_t size)
{
	callArgs_.reserve(size);
}

void RenderBatchHandler::Render(SDL_Renderer* renderer)
{
	if (callArgs_.empty() || batches_.empty())
		return;

	// Close the last batch
	batches_.back().batchEndIndex = callArgs_.size();

	size_t argsIdx = 0;
	for (auto& [texture, batchEndIdx] : batches_)
	{
		for (; argsIdx < batchEndIdx; ++argsIdx)
		{
			const auto& args = callArgs_[argsIdx];

			modTracker_.Update(texture, args.mods);

			SDL_RenderCopyEx(renderer, texture, &args.srcRect,
							 &args.destRect, args.rotationAngle,
							 GetRotationCenter(args), args.flip);
		}

		modTracker_.Reset(texture);
	}
}

SDL_Rect MakeScreenRect(const Camera& camera, const Transform& transform, 
						int w, int h, SDL_FPoint offset)
{
	SDL_Point screenPos = camera.WorldToScreen<SDL_Point>(transform.position + offset);

	float scaledW = w * transform.scale.x;
	float scaledH = h * transform.scale.y;

	return SDL_Rect{
		static_cast<int>(screenPos.x - (scaledW / 2.0f)),
		static_cast<int>(screenPos.y - (scaledH / 2.0f)),
		static_cast<int>(scaledW),
		static_cast<int>(scaledH)
	};
}
