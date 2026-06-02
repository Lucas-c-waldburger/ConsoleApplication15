#pragma once
#include "../sdl/SDLUtils.h"
#include <numbers>

namespace detail {
	template <SDLPointType T> struct value_type_for_point;
	template <> struct value_type_for_point<SDL_Point> { using type = int; };
	template <> struct value_type_for_point<SDL_FPoint> { using type = float; };

	template <SDLRectType T> struct point_type_for_rect;
	template <> struct point_type_for_rect<SDL_Rect> { using type = SDL_Point; };
	template <> struct point_type_for_rect<SDL_FRect> { using type = SDL_FPoint; };
} // detail

template <SDLPointType T>
using value_type_for_point_t = detail::value_type_for_point<T>::type;

template <SDLRectType T>
using point_type_for_rect_t = detail::point_type_for_rect<T>::type;

template <SDLRectType T>
using value_type_for_rect_t = value_type_for_point_t<point_type_for_rect_t<T>>;


template <SDLPointType P, SDLPointType C>
inline constexpr SDL_FPoint RotatePointRadians(P point, C center, float angleRadians)
{
	const float s = std::sin(angleRadians);
	const float c = std::cos(angleRadians);

	const float dx = static_cast<float>(point.x) - static_cast<float>(center.x);
	const float dy = static_cast<float>(point.y) - static_cast<float>(center.y);

	return {
		center.x + dx * c - dy * s,
		center.y + dx * s + dy * c
	};
}

template <SDLPointType P, SDLPointType C>
inline constexpr SDL_FPoint RotatePointDegrees(P point, C center, float angleDegrees)
{
	float angleRadians = angleDegrees * (std::numbers::pi_v<float> / 180.0f);

	return RotatePointRadians(point, center, angleRadians);
}


class Projection
{
public:
	template <SDLPointType PointOut, SDLPointType PointIn = PointOut>
	static constexpr PointOut WorldToScreen(PointIn worldPoint, SDL_FPoint cameraWorldPos,
											Dimensions<float> vpDimensions, float zoom = 1.0f,
											float angleDegrees = 0.0f) noexcept
	{
		using ValueType = value_type_for_point_t<PointOut>;

		auto offset = GetCameraOffset<SDL_FPoint>(cameraWorldPos, vpDimensions, zoom);

		// Apply offset and zoom
		SDL_FPoint screenPoint = {
			(worldPoint.x - offset.x) * zoom,
			(worldPoint.y - offset.y) * zoom
		};

		// Rotate around center of screen if needed
		if (angleDegrees != 0.0f)
		{
			SDL_FPoint center = { vpDimensions.w / 2.0f, vpDimensions.h / 2.0f };

			screenPoint = RotatePointDegrees(screenPoint, center, angleDegrees);
		}

		return {
			static_cast<ValueType>(screenPoint.x),
			static_cast<ValueType>(screenPoint.y)
		};

	}

	template <SDLRectType RectOut, SDLRectType RectIn = RectOut>
	static constexpr RectOut WorldToScreen(RectIn worldRect, SDL_FPoint cameraWorldPos,
										   Dimensions<float> vpDimensions, float zoom = 1.0f,
										   float angleDegrees = 0.0f) noexcept
	{
		// Map the rectangle by transforming its center. This ensures a consistent
		// rotation around the viewport center and prevents per-rect offset artifacts.
		using PointType = point_type_for_rect_t<RectOut>;
		using ValueType = value_type_for_point_t<PointType>;

		// Compute world-space center of the rect
		SDL_FPoint worldCenter{
			static_cast<float>(worldRect.x) + static_cast<float>(worldRect.w) * 0.5f,
			static_cast<float>(worldRect.y) + static_cast<float>(worldRect.h) * 0.5f
		};

		// Transform center to screen space (this applies rotation around viewport center)
		PointType screenCenter = WorldToScreen<PointType>(worldCenter, cameraWorldPos, vpDimensions, zoom, angleDegrees);

		// Scaled size
		float scaledW = static_cast<float>(worldRect.w) * zoom;
		float scaledH = static_cast<float>(worldRect.h) * zoom;

		return {
			static_cast<ValueType>(static_cast<float>(screenCenter.x) - (scaledW * 0.5f)),
			static_cast<ValueType>(static_cast<float>(screenCenter.y) - (scaledH * 0.5f)),
			static_cast<ValueType>(scaledW),
			static_cast<ValueType>(scaledH)
		};
	}


	template <SDLPointType PointOut, SDLPointType PointIn = PointOut>
	static constexpr PointOut ScreenToWorld(PointIn screenPoint, SDL_FPoint cameraWorldPos,
											Dimensions<float> vpDimensions, float zoom = 1.0f,
											float angleDegrees = 0.0f) noexcept
	{
		using ValueType = value_type_for_point_t<PointOut>;

		// Reverse rotate if needed
		SDL_FPoint unrotated = (angleDegrees != 0.0f)
			? RotatePointDegrees(screenPoint, SDL_FPoint{ vpDimensions.w / 2.0f, vpDimensions.h / 2.0f }, -angleDegrees)
			: SDL_FPoint{ static_cast<float>(screenPoint.x), static_cast<float>(screenPoint.y) };

		auto offset = GetCameraOffset<SDL_FPoint>(cameraWorldPos, vpDimensions, zoom);

		return {
			static_cast<ValueType>((unrotated.x / zoom) + offset.x),
			static_cast<ValueType>((unrotated.y / zoom) + offset.y)
		};
	}


	template <SDLRectType RectOut, SDLRectType RectIn = RectOut>
	static constexpr RectOut ScreenToWorld(RectIn screenRect, SDL_FPoint cameraWorldPos,
										   Dimensions<float> vpDimensions, float zoom = 1.0f,
										   float angleDegrees = 0.0f) noexcept
	{
		// Inverse of the rect mapping above: transform the screen rect center back to world,
		// and compute world top-left from the unscaled world size.
		using PointType = point_type_for_rect_t<RectOut>;
		using ValueType = value_type_for_point_t<PointType>;

		// Screen-space center
		SDL_FPoint screenCenter{
			static_cast<float>(screenRect.x) + static_cast<float>(screenRect.w) * 0.5f,
			static_cast<float>(screenRect.y) + static_cast<float>(screenRect.h) * 0.5f
		};

		// Convert center back to world space
		PointType worldCenter = ScreenToWorld<PointType>(screenCenter, cameraWorldPos, vpDimensions, zoom, angleDegrees);

		// World-space size
		float worldW = static_cast<float>(screenRect.w) / zoom;
		float worldH = static_cast<float>(screenRect.h) / zoom;

		return {
			static_cast<ValueType>(static_cast<float>(worldCenter.x) - (worldW * 0.5f)),
			static_cast<ValueType>(static_cast<float>(worldCenter.y) - (worldH * 0.5f)),
			static_cast<ValueType>(worldW),
			static_cast<ValueType>(worldH)
		};
	}


private:
	template <SDLPointType P>
	static constexpr P GetCameraOffset(SDL_FPoint cameraWorldPos, Dimensions<float> vpDimensions,
									   float zoom = 1.0f) noexcept
	{
		using ValueType = value_type_for_point_t<P>;

		return {
			static_cast<ValueType>(cameraWorldPos.x - (vpDimensions.w / (2.0f * zoom))),
			static_cast<ValueType>(cameraWorldPos.y - (vpDimensions.h / (2.0f * zoom)))
		};
	}

	Projection() = default;
};