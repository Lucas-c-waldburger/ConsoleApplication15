#include "CameraControlUtility.h"

#if IMGUI_ENABLED
#include "../../camera/Camera.h"
#include "GuiMouse.h"

namespace ui {

static constexpr Dimensions<int> kScreenEdgeScrollBuffer = { 50, 50 };
static constexpr float kScrollMaxSpeed = 850.0f;
static constexpr float kScrollMaxDistance = 450.0f;
static constexpr float kZoomIncrement = 0.10f;

void CameraControlUtility::UpdateScroll(Camera& cam, float dt)
{
	if (scrollOffset_.has_value())
	{
		if (GuiMouse::IsMiddleClicked() || GuiMouse::IsLeftClicked() ||
			GuiMouse::IsRightClicked())
		{
			SDL_SetCursor(SDL_GetDefaultCursor());
			scrollOffset_.reset();
		}
		else
		{
			HandleFreeScroll(cam, dt);
		}
	}
	else if (GuiMouse::IsMiddleClicked() && !GuiMouse::InsideEditorWindow())
	{
		SDL_SetCursor(scrollCursor_.get());
		scrollOffset_.emplace(0.0f, 0.0f);
	}
}

void CameraControlUtility::UpdateZoom(Camera& cam)
{
	if (GuiMouse::IsWheelScrolled() && !GuiMouse::InsideEditorWindow())
	{
		float newScale = cam.GetZoomScale() + (GuiMouse::GetScrollY() > 0.0f 
			? kZoomIncrement : -kZoomIncrement);
		if (newScale < 0.1f)
		{
			newScale = 0.1f;
		}

		cam.SetZoomScale(newScale);
	}
}

void CameraControlUtility::HandleFreeScroll(Camera& cam, float dt)
{
	assert(scrollOffset_.has_value());

	*scrollOffset_ += GuiMouse::GetRelativePosition();

	auto dir = *scrollOffset_;
	float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (dist > 0.0f)
	{
		dir.x /= dist;
		dir.y /= dist;
	}

	float t = std::min(dist / kScrollMaxDistance, 1.0f);
	float speed = t * kScrollMaxSpeed;

	cam.Pan({
		dir.x * speed * static_cast<float>(dt),
		dir.y * speed * static_cast<float>(dt)
	});
}

Result<Void> CameraControlUtility::Init()
{
	scrollCursor_ = MakeUniqueCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	if (!scrollCursor_)
	{
		return MAKE_ERROR("Failed to create scroll cursor");
	}

	return kVoid;
}

void CameraControlUtility::Reset()
{
	SDL_SetCursor(SDL_GetDefaultCursor());
	scrollOffset_.reset();
}


} // ui

#endif