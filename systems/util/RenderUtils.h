#pragma once
#include <SDL_rect.h>

struct Transform;
class Camera;

SDL_Rect MakeTransformedRect(const Transform& transform, int w, int h,
							 SDL_FPoint offset);

SDL_Rect MakeScreenRect(const Camera& camera, const Transform& transform, int w, int h,
						SDL_FPoint offset = { 0.0f, 0.0f });