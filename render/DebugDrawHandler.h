#pragma once
#include <vector>
#include <unordered_set>
#include <SDL_pixels.h>
#include <SDL_rect.h>
#include "../core/Handle.h"

class Camera;
class B2Chain;
struct RenderProfile;
struct Collider;
struct SDL_Renderer;

class DebugDrawHandler
{
public:
	static constexpr size_t kDefaultPointReserveSize = 512;

	DebugDrawHandler() { debugDrawPoints_.reserve(kDefaultPointReserveSize); }
	explicit DebugDrawHandler(size_t reserveSize) { debugDrawPoints_.reserve(reserveSize); }

	void Clear();
	void Reserve(size_t size);
	void Reset(size_t newSize);
	void AddBoundingBox(SDL_Rect renderRect, float rotation, const RenderProfile& profile);
	void AddColliderShape(const Camera& camera, const Collider& collider,
						  const RenderProfile& profile);
	void Draw(SDL_Renderer* renderer);

private:
	struct ShapeResource
	{
		SDL_Color color = { 255, 255, 255, 255 };
		size_t shapeEndIndex = 0;
	};

	std::vector<ShapeResource> shapes_;
	std::vector<SDL_FPoint> debugDrawPoints_;
	std::unordered_set<Handle<B2Chain>> handledChains_;
};