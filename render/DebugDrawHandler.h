#pragma once
#include <vector>
#include <SDL_pixels.h>
#include <SDL_rect.h>

class Camera;
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
};