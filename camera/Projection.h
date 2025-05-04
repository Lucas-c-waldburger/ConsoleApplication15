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
		using PointType = point_type_for_rect_t<RectOut>;
		using ValueType = value_type_for_point_t<PointType>;

		const PointType topLeftWorld = {
			static_cast<ValueType>(worldRect.x),
			static_cast<ValueType>(worldRect.y)
		};

		const PointType topLeftScreen = WorldToScreen<PointType>(topLeftWorld, cameraWorldPos,
			vpDimensions, zoom, angleDegrees);

		return {
			topLeftScreen.x,
			topLeftScreen.y,
			static_cast<ValueType>(worldRect.w * zoom),
			static_cast<ValueType>(worldRect.h * zoom)
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
		using PointType = point_type_for_rect_t<RectOut>;
		using ValueType = value_type_for_point_t<PointType>;

		const PointType topLeftScreen = {
			static_cast<ValueType>(screenRect.x),
			static_cast<ValueType>(screenRect.y)
		};

		const PointType topLeftWorld = ScreenToWorld<PointType>(topLeftScreen, cameraWorldPos,
			vpDimensions, zoom, angleDegrees);

		return {
			topLeftWorld.x,
			topLeftWorld.y,
			static_cast<ValueType>(screenRect.w / zoom),
			static_cast<ValueType>(screenRect.h / zoom)
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