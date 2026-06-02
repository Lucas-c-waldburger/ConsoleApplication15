#pragma once
#include <algorithm>
#include "Projection.h"
#include "../core/commonObjects.h"
#include <numbers>
#include <array>


class Camera
{
public:
	class Viewport
	{
	public:
		Viewport(Dimensions<float> size, std::array<SDL_FPoint, 4>&& corners, SDL_FRect bbox) : 
			size_(size), corners_(std::move(corners)), boundingBox_(bbox) {}

		Dimensions<float> GetSize() const { return size_; }
		const std::array<SDL_FPoint, 4>& GetCorners() const { return corners_; }
		SDL_FRect GetBoundingBox() const { return boundingBox_; }
		SDL_FPoint GetCenter() const;

		template <SDLRectType T>
		bool IntersectsBoundingBox(T rect, 
			Dimensions<float> padding = { 0.0f, 0.0f }) const noexcept
		{
			return RectsIntersect(GetPaddedBoundingBox(padding), rect);
		}

		template <SDLPointType T>
		bool PointInsideBoundingBox(T p, 
			Dimensions<float> padding = { 0.0f, 0.0f }) const noexcept
		{ 
			return PointInsideRect(GetPaddedBoundingBox(padding), p);
		}

	private:
		SDL_FRect GetPaddedBoundingBox(Dimensions<float> padding) const noexcept;

		Dimensions<float> size_;
		std::array<SDL_FPoint, 4> corners_;
		SDL_FRect boundingBox_;
	};

	Camera() = default;
	explicit Camera(Dimensions<float> vpSize) : viewportSize_(vpSize) {}
	Camera(Dimensions<float> vpSize, Range<SDL_FPoint> bounds) : viewportSize_(vpSize), bounds_(bounds) {}
	Camera(Dimensions<float> vpSize, Range<SDL_FPoint> bounds, float zoom, float rotDeg) : 
		viewportSize_(vpSize), bounds_(bounds), zoomScale_(zoom), rotationDegrees_(rotDeg) {}
	~Camera() = default;

	Dimensions<float> GetViewportSize() const { return viewportSize_; }
	void SetViewportSize(Dimensions<float> newSize) 
	{ 
		viewportSize_ = newSize;
	}

	Viewport GetViewport() const;

	std::array<SDL_FPoint, 4> GetViewportCorners() const;

	template <SDLRectType R = SDL_FRect>
	R GetViewportBoundingBox() const
	{
		return GetBoundingBoxForCorners<R>(GetViewportCorners());
	}

	SDL_FPoint GetPosition() const { return worldPosition_; }
	void SetPosition(SDL_FPoint newPos, bool clamp = true);

	Range<SDL_FPoint> GetBounds() const { return bounds_; }
	void SetBounds(SDL_FPoint min, SDL_FPoint max) { bounds_.min = min; bounds_.max = max; }

	float GetZoomScale() const { return zoomScale_; }
	void SetZoomScale(float newZoom) { zoomScale_ = newZoom; }

	float GetRotation() const { return rotationDegrees_; }
	void SetRotation(float newRotDeg) { rotationDegrees_ = newRotDeg; }

	template <typename T, typename U = T>
	T WorldToScreen(U pointOrRect) const
	{
		return Projection::WorldToScreen<T>(pointOrRect, worldPosition_, viewportSize_, 
										    zoomScale_, rotationDegrees_);
	}

	template <typename T, typename U = T>
	T ScreenToWorld(U pointOrRect) const
	{
		return Projection::ScreenToWorld<T>(pointOrRect, worldPosition_, viewportSize_, 
											zoomScale_, rotationDegrees_);
	}

	SDL_FPoint ClampToBounds(SDL_FPoint pos) const;

	void Pan(SDL_FPoint delta);

private:
	template <SDLRectType R = SDL_FRect>
	static R GetBoundingBoxForCorners(std::array<SDL_FPoint, 4> corners)
	{
		float minX = corners[0].x;
		float minY = corners[0].y;
		float maxX = corners[0].x;
		float maxY = corners[0].y;

		for (int i = 1; i < 4; ++i)
		{
			minX = std::min(minX, corners[i].x);
			minY = std::min(minY, corners[i].y);
			maxX = std::max(maxX, corners[i].x);
			maxY = std::max(maxY, corners[i].y);
		}

		using ValueType = value_type_for_rect_t<R>;

		return R{
			static_cast<ValueType>(minX),
			static_cast<ValueType>(minY),
			static_cast<ValueType>(maxX - minX),
			static_cast<ValueType>(maxY - minY)
		};
	}

	SDL_FPoint worldPosition_ = { 0.0f, 0.0f };
	Dimensions<float> viewportSize_;
	Range<SDL_FPoint> bounds_ = { .min = { -INFINITY, -INFINITY },
								  .max = { INFINITY, INFINITY } };
	float zoomScale_ = 1.0f;
	float rotationDegrees_ = 0.0f;
};