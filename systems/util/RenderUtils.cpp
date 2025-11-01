#include "RenderUtils.h"
#include "../../camera/Camera.h"
#include "../../components/TransformComponent.h"
#include <SDL_render.h>

SDL_Rect MakeTransformedRect(const Transform& transform, int w, int h,
							 SDL_FPoint offset)
{
	float scaledW = w * transform.scale.x;
	float scaledH = h * transform.scale.y;

	return SDL_Rect{
		static_cast<int>((transform.position.x + offset.x) - (scaledW / 2.0f)),
		static_cast<int>((transform.position.y + offset.y) - (scaledH / 2.0f)),
		static_cast<int>(scaledW),
		static_cast<int>(scaledH)
	};
}

SDL_Rect MakeScreenRect(const Camera& camera, const Transform& transform,
						int w, int h, SDL_FPoint offset)
{
	SDL_Point screenPos = camera.WorldToScreen<SDL_Point>(transform.position + offset);

	float scaledW = w * transform.scale.x;
	float scaledH = h * transform.scale.y;

	return SDL_Rect{
		static_cast<int>(screenPos.x - (scaledW / 2.0f)),
		static_cast<int>(screenPos.y - (scaledH / 2.0f)),
		static_cast<int>(scaledW),
		static_cast<int>(scaledH)
	};
}
