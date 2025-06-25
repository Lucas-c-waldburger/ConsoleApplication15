#pragma once
#include "../sdl/SDLUtils.h"
#include "../core/Result.h"
#include "../atlas/AtlasManager.h"
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
	// TODO: Attach stores to render system via Scene
	void Update(SDL_Renderer* renderer, const Camera& camera, const impl::TextureManager& atlasStore);
	
private:	
	struct RenderGlyphsArgs
	{
		SDL_Renderer* renderer = nullptr;
		const GlyphAtlas* glyphAtlas;
		std::string_view text;
		SDL_FPoint scale = { 0.0f, 0.0f };
		int numNewlines = 0;
		int startY = 0;
		int startX = 0;
	};



	static float GetScaleToFitFactor(const RenderGlyphsArgs& args, int totalHeight,
									 int boundingWidth, int boundingHeight);
};

class NewRenderSystem : public System
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


//class NewRenderSystem : public System
//{
//public:
//	void Update(SDL_Renderer* renderer, const Camera& camera, const TextureRepository& textureRepo);
//
//private:
//	struct SpriteRenderContext
//	{
//		SDL_Renderer* renderer = nullptr;
//		const Camera* camera = nullptr;
//		const TextureRepository* textureRepo = nullptr;
//		std::vector<SDL_FPoint> debugDrawPoints;
//		SDL_Color currentDrawColor = { 0, 0, 0, 255 };
//	};
//
//	struct QueueKeyOld
//	{
//		Entity_t entityId = kInvalidEntity;
//		int drawOrder = 0;
//	};
//
//	struct QueueKey
//	{
//		Entity_t entityId = kInvalidEntity;
//		ComponentSignature renderableType = 0;
//		int drawOrder = 0;
//	};
//
//	using RenderOpQueue = std::vector<QueueKey>;
//
//	using RenderOperation = fu2::unique_function<void(Entity_t, SpriteRenderContext&)>;
//	using RenderOperationQueue = std::vector<std::pair<QueueKeyOld, RenderOperation>>;
//
//	static RenderOperation MakeSpriteRenderOperation();
//	static RenderOperation MakeTextRenderOperation();
//
//	SpriteRenderContext context_;
//	RenderOperationQueue operationQueue_;
//	RenderOpQueue opQueue_;
//};