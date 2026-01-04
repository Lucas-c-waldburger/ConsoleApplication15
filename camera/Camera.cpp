#include "Camera.h"

namespace {

//SDL_FPoint ClampToCameraBounds(SDL_FPoint pos, Range<SDL_FPoint> bounds, Dimensions<float> vpDimensions)
//{
//	return {
//		std::clamp(pos.x, bounds.min.x + vpDimensions.w / 2.0f,
//					bounds.max.x - vpDimensions.w / 2.0f),
//		std::clamp(pos.y, bounds.min.y + vpDimensions.h / 2.0f,
//					bounds.max.y - vpDimensions.h / 2.0f)
//	};
//}
SDL_FPoint ClampToCameraBounds(
	SDL_FPoint pos,
	Range<SDL_FPoint> bounds,
	Dimensions<float> worldViewportSize
)
{
	return {
		std::clamp(
			pos.x,
			bounds.min.x + worldViewportSize.w / 2.0f,
			bounds.max.x - worldViewportSize.w / 2.0f
		),
		std::clamp(
			pos.y,
			bounds.min.y + worldViewportSize.h / 2.0f,
			bounds.max.y - worldViewportSize.h / 2.0f
		)
	};
}

SDL_FPoint GetCenterFromCorners(const std::array<SDL_FPoint, 4>& corners) 
{
	SDL_FPoint center{ 0.0f, 0.0f };
	for (const auto& corner : corners)
	{
		center.x += corner.x;
		center.y += corner.y;
	}

	center.x /= 4.0f;
	center.y /= 4.0f;

	return center;
}

} // unnamed

// Viewport
SDL_FPoint Camera::Viewport::GetCenter() const
{
	return GetCenterFromCorners(corners_);
}

SDL_FRect 
Camera::Viewport::GetPaddedBoundingBox(Dimensions<float> padding) const noexcept
{
	return {
		boundingBox_.x - (padding.w / 2.0f),
		boundingBox_.y - (padding.h / 2.0f),
		boundingBox_.w + padding.w,
		boundingBox_.h + padding.h
	};
}


// Camera
void Camera::SetPosition(SDL_FPoint newPos, bool clamp)
{
	newPos.x = std::round(newPos.x);
	newPos.y = std::round(newPos.y);

	if (!clamp)
	{
		worldPosition_ = newPos;
		return;
	}

	Dimensions<float> worldViewportSize{
		viewportSize_.w / zoomScale_,
		viewportSize_.h / zoomScale_
	};

	worldPosition_ = ClampToCameraBounds(
		newPos,
		bounds_,
		worldViewportSize
	);
}

Camera::Viewport Camera::GetViewport() const
{
	auto corners = GetViewportCorners();

	Dimensions<float> worldViewportSize{
		viewportSize_.w / zoomScale_,
		viewportSize_.h / zoomScale_
	};

	auto bbox = GetBoundingBoxForCorners(corners);

	return Viewport{
		worldViewportSize,
		std::move(corners),
		bbox
	};
}


std::array<SDL_FPoint, 4> Camera::GetViewportCorners() const
{
	const float zoomedW = viewportSize_.w / zoomScale_;
	const float zoomedH = viewportSize_.h / zoomScale_;

	const float hw = zoomedW / 2.0f;
	const float hh = zoomedH / 2.0f;

	// Unrotated corners relative to the center
	std::array<SDL_FPoint, 4> corners = {
		SDL_FPoint{ -hw, -hh }, // top-left
		SDL_FPoint{  hw, -hh }, // top-right
		SDL_FPoint{  hw,  hh }, // bottom-right
		SDL_FPoint{ -hw,  hh }  // bottom-left
	};

	// Convert degrees to radians
	const float angleRadians = rotationDegrees_ * (std::numbers::pi_v<float> / 180.0f);
	const float cosA = std::cos(angleRadians);
	const float sinA = std::sin(angleRadians);

	// Apply rotation and translate to world position
	for (auto& corner : corners)
	{
		float x = corner.x;
		float y = corner.y;
		corner.x = worldPosition_.x + (x * cosA - y * sinA);
		corner.y = worldPosition_.y + (x * sinA + y * cosA);
	}

	return corners;
}

SDL_FPoint Camera::ClampToBounds(SDL_FPoint pos) const
{
	return ClampToCameraBounds(pos, bounds_, viewportSize_);
}

void Camera::Pan(SDL_FPoint delta)
{
	worldPosition_.x += std::round(delta.x);
	worldPosition_.y += std::round(delta.y);
}