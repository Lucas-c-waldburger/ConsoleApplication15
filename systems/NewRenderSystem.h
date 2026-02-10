#pragma once
#include <vector>
#include "System.h"
#include "../render/RenderBatchHandler.h"
#include "../render/DebugDrawHandler.h"
#include "RenderablePreProcessorSystem.h"

class Camera;
class TextureRepository;
struct SDL_Renderer;

class NewRenderSystem : public System
{
public:
	NewRenderSystem() = default;

	void Update(SDL_Renderer* renderer, const Camera& camera,
				const TextureRepository& textureRepo);

private:
	RenderablePreProcessor renderablePreProcessor_;
	RenderBatchHandler renderBatchHandler_;
	DebugDrawHandler debugDrawHandler_;
};