#pragma once
#include "TextureMods.h"

struct TextureModTracker
{
	void Update(SDL_Texture* texture, const TextureMods& newMods);
	void Reset(SDL_Texture* texture);

	TextureMods trackedMods;
};

struct RenderCallArgs
{
	SDL_Rect srcRect = { 0, 0, 0, 0 };
	SDL_Rect destRect = { 0, 0, 0, 0 };
	double rotationAngle = 0.0;
	std::optional<SDL_Point> rotationCenter;
	TextureMods mods;
	SDL_RendererFlip flip;
};

class RenderBatchHandler
{
public:
	RenderBatchHandler() = default;

	void StartRenderBatch(SDL_Texture* texture);
	void PushBack(RenderCallArgs&& args) { callArgs_.push_back(std::move(args)); }
	void Clear();
	void Reserve(size_t size);
	void Render(SDL_Renderer* renderer);

private:
	struct BatchResource
	{
		SDL_Texture* texture = nullptr;
		size_t batchEndIndex = 0;
	};

	std::vector<BatchResource> batches_;
	std::vector<RenderCallArgs> callArgs_;
	TextureModTracker modTracker_;
};