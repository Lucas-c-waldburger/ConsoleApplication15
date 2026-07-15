#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <SDL_rect.h>

class Entity;
class Camera;

namespace ui {

class EntityDragUtility
{
public:
	void Update(Entity& e, const Camera& cam);	
	void Reset();
	bool IsDragging() const { return isDragging_; }

private:
	void SetOverrideDelta(const Entity& e, SDL_FPoint mouseAbsPos);
	void AdjustEntityPosition(Entity& e);

	bool isDragging_ = false;
	SDL_FPoint overrideDelta_ = { 0.0f, 0.0f };
};

} // ui

#endif