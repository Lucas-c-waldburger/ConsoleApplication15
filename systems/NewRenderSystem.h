#pragma once
#include <vector>
#include "System.h"
#include "../render/RenderBatchHandler.h"
#include "../render/DebugDrawHandler.h"
#include "GlyphFormattingSystem.h"

class Camera;
class NewTextureRepository;
struct SDL_Renderer;

//class NewRenderSystem : public System
//{
//public:
//	NewRenderSystem() = default;
//
//	void Update(SDL_Renderer* renderer, const Camera& camera, 
//		        const NewTextureRepository& textureRepo);
//
//private:
//	RenderablePreProcessor renderablePreProcessor_;
//	RenderBatchHandler renderBatchHandler_;
//	DebugDrawHandler debugDrawHandler_;
//};

class NewRenderSystem : public System
{
public:
	NewRenderSystem() = default;

	void Update(SDL_Renderer* renderer, const Camera& camera,
		const NewTextureRepository& textureRepo);

private:
	RenderablePreProcessor renderablePreProcessor_;
	RenderBatchHandler renderBatchHandler_;
	DebugDrawHandler debugDrawHandler_;
};