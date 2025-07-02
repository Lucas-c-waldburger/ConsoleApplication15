#pragma once
#include "../sdl/SDLUtils.h"
#include "../core/Result.h"
#include "../atlas/TextureRepository.h"
#include "../deps/function2/function2.hpp"
#include "../ecs/EntityT.h"
#include "../physics/B2Shape.h"
#include "System.h"
#include <span>
#include <numeric>
#include <typeindex>

class GlyphAtlas;
class Camera;

struct Transform;
struct SpriteRenderable;
struct TextRenderable;
struct RenderProfile;

class RenderSystem : public System
{
public:
	struct TextRenderParams
	{
		std::string_view text;
		Dimensions<int> bounds = { 0, 0 };
		SDL_FPoint scale = { 0.0f, 0.0f };
		SDL_Point start = { 0, 0 };
		int numNewlines = 0;
		int fontHeight = 0;
		int totalHeight = 0;
	};

	struct RenderContext
	{
		SDL_Renderer* renderer = nullptr;
		const TextureRepository* textureRepo = nullptr;
		const Camera* camera = nullptr;
		std::vector<SDL_FPoint> debugDrawPoints;
		SDL_Color currentDrawColor = { 0, 0, 0, 255 };
	};

	void Update(SDL_Renderer* renderer, const Camera& camera, const TextureRepository& textureRepo);

private:
	void UpdateRenderContext(SDL_Renderer* renderer, const TextureRepository& textureRepo,
						     const Camera& camera);

	void RenderSprite(const SpriteRenderable& spriteRenderable, const Transform& transform,
					  const RenderProfile& renderProfile);
	void RenderText(TextRenderable& textRenderable, const Transform& transform,
				    const RenderProfile& renderProfile);

	RenderContext context_;
};
