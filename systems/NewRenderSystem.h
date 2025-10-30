#pragma once
#include "System.h"
#include <vector>
#include "util/RenderUtils.h"

class Camera;
class NewTextureRepository;
struct SDL_Renderer;

class NewRenderSystem : public System
{
public:
	void Update(SDL_Renderer* renderer, const Camera& camera, 
		        const NewTextureRepository& textureRepo);

private:
	RenderBatchHandler renderBatchHandler_;
};