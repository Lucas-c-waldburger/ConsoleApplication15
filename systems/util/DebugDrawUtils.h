#pragma once
#include <vector>
#include <array>
#include <SDL_rect.h>
#include <SDL_pixels.h>

class Camera;
class B2Shape;
struct SDL_Renderer;
struct Collider;
struct RenderProfile;

//class DebugDrawHandler
//{
//public:
//	DebugDrawHandler() = default;
//	explicit DebugDrawHandler(SDL_Color orig) : originalDrawColor_(orig) {}
//
//	void Reset(SDL_Color startingColor);
//
//	void DrawBoundingBox(SDL_Renderer* renderer, SDL_Rect renderRect,
//						 float rotation, SDL_Color color);
//	void DrawColliderShape(SDL_Renderer* renderer, const Camera& camera, 
//						   const Collider& collider, SDL_Color color);
//
//private:
//	std::vector<SDL_FPoint> debugDrawPoints_;
//	SDL_Color originalDrawColor_;
//	SDL_Color currentDrawColor_;
//};



namespace util {

std::vector<SDL_FPoint> MakeCirclePerimeterPoints(SDL_FPoint center, float radius);

void DrawB2ColliderShape(const Camera& camera, SDL_Renderer* renderer, 
						 const B2Shape& shapeData);
void DrawB2ColliderShape(const Camera& camera, SDL_Renderer* renderer,
						 const Collider& collider);

constexpr void ExtractRectPoints(SDL_Rect rect, std::array<SDL_FPoint, 5>& points);
void ExtractRectPoints(SDL_Rect rect, std::vector<SDL_FPoint>& points);

void RotateRectPoints(std::array<SDL_FPoint, 5>& points, SDL_FPoint center, float angleDegrees);
void RotateRectPoints(std::vector<SDL_FPoint>& points, SDL_FPoint center, float angleDegrees);

SDL_Rect ComputeBoundingBox(const std::vector<SDL_Rect>& rects);

} // util