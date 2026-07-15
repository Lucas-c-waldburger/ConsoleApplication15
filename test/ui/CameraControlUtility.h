#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../sdl/SDLUtils.h"
#include "../../core/commonObjects.h"
#include "../../core/Result.h"

class Camera;

namespace ui {

class CameraControlUtility
{
public:
	Result<Void> Init();

	void UpdateScroll(Camera& cam, float dt);
	void UpdateZoom(Camera& cam);
	void Reset();
	bool IsScrolling() const { return scrollOffset_.has_value(); }

private:
	void HandleFreeScroll(Camera& cam, float dt);

	bool isActive_ = false;
	bool isScrolling_ = false;
	SDL_FPoint lastMousePos_ = { 0.0f, 0.0f };
	UniqueCursorPtr scrollCursor_;
	std::optional<SDL_FPoint> scrollOffset_;
};

} // ui

#endif