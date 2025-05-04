#pragma once
#include "../sdl/SDLUtils.h"
#include "commonObjects.h"
#include "../components/TransformComponent.h"
#include "../components/ColliderComponent.h"

template <SDLRectType Rect>
static constexpr Rect ToRect(const SDL_FPoint p, const Dimensions<float> dims, float scale)
{
	float scaledW = dims.w * scale.x;
	float scaledH = dims.h * scale.y;

	using ValueType = std::remove_cvref_t<decltype(Rect::x)>;

	return Rect{
		static_cast<ValueType>(p.x - (scaledW / 2.0f)),
		static_cast<ValueType>(p.y - (scaledH / 2.0f)),
		static_cast<ValueType>(scaledW),
		static_cast<ValueType>(scaledH)
	};
}

template <SDLRectType Rect>
static constexpr std::pair<SDL_FPoint, Dimensions<float>> FromRect(const Rect r)
{
	return std::make_pair(
		SDL_FPoint{ static_cast<float>(r.x + (r.w / 2.0f)),
					static_cast<float>(r.y + (r.h / 2.0f)) },
		Dimensions<float>{ static_cast<float>(r.w), static_cast<float>(r.h) }
	);
}