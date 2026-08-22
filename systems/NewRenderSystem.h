#pragma once
#include <vector>
#include "System.h"
#include "../render/RenderBatchHandler.h"
#include "../render/DebugDrawHandler.h"
#include "RenderablePreProcessorSystem.h"

class Camera;
class TextureRepository;
class Entity;
struct SDL_Renderer;

class NewRenderSystem : public System
{
public:
	NewRenderSystem() = default;

	void Update(SDL_Renderer* renderer, const Camera& camera,
				const TextureRepository& textureRepo);
	void Update(std::vector<Entity>& entities,
				SDL_Renderer* renderer, const Camera& camera,
				const TextureRepository& textureRepo);

private:
	RenderablePreProcessor renderablePreProcessor_;
	RenderBatchHandler renderBatchHandler_;
	DebugDrawHandler debugDrawHandler_;
};