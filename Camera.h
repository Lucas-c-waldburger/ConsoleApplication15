#pragma once
#include <algorithm>
#include <optional>
#include "sdl/SDLUtils.h"
#include "core/commonObjects.h"
#include "ecs/EntityT.h"
#include "ecs/Ecs.h"

class Camera
{
public:
	//struct Viewport : public SDL_Rect
	//{
	//	constexpr SDL_Point GetCenter() const
	//	{
	//		return { x + (w / 2), y + (h / 2) };
	//	}
	//};

	struct FollowTarget
	{
		Entity_t entityId = kInvalidEntity;
		SDL_FPoint offset = { 0.0f, 0.0f };
		float followSpeed = 5.0f;

		std::optional<SDL_FPoint> GetTargetPosition()
		{
			if (entityId == kInvalidEntity) 
			{ 
				return std::nullopt;
			}

			auto target = ECS::GetEntityByID(entityId);
			if (!target.IsValid())
			{
				LOG_WARNING("Target entity invalid, removing");
				entityId = kInvalidEntity;

				return std::nullopt;
			}
			if (!target.HasComponent<Spatial>())
			{
				LOG_WARNING("Target entity did not have spatial component, removing");
				entityId = kInvalidEntity;

				return std::nullopt;
			}

			auto& spatial = target.GetComponent<Spatial>();

			return SDL_FPoint{ spatial.position.x, spatial.position.y };
		}
	};

	SDL_Rect GetViewport() const
	{ 
		return { 
			static_cast<int>(vpPosition_.x - (vpDimensions_.w / 2.0f)), 
			static_cast<int>(vpPosition_.y - (vpDimensions_.h / 2.0f)),
			static_cast<int>(vpDimensions_.w), 
			static_cast<int>(vpDimensions_.h)
		};
	}

	void SetPosition(SDL_FPoint newPos) { vpPosition_ = ClampToBounds(newPos); }
	SDL_FPoint GetPosition() const { return vpPosition_;  }
	void SetDimensions(Dimensions<float> newDims) { vpDimensions_ = newDims; }
	Dimensions<float> GetDimensions() const { return vpDimensions_; }

	void Move(SDL_FPoint d) { vpPosition_ = ClampToBounds({ vpPosition_.x + d.x, vpPosition_.y + d.y }); }

	void SetFollowTarget(const Entity& entity, SDL_FPoint offset = { 0.0f, 0.0f }, float speed = 5.0f)
	{
		followTarget_.entityId = entity.GetID();
		followTarget_.offset = offset;
		followTarget_.followSpeed = speed;
	}
	const FollowTarget& GetFollowTarget() { return followTarget_; }


	SDL_FPoint GetPredictedPosition(float deltaTime)
	{
		auto targetPos = followTarget_.GetTargetPosition();
		if (!targetPos.has_value())
		{
			return vpPosition_;
		}

		SDL_FPoint predicted = vpPosition_;

		predicted.x += (targetPos->x + followTarget_.offset.x - predicted.x) *
			followTarget_.followSpeed * deltaTime;
		predicted.y += (targetPos->y + followTarget_.offset.y - predicted.y) *
			followTarget_.followSpeed * deltaTime;

		predicted = ClampToBounds(predicted);

		return predicted;
	}

	void Update(float deltaTime) 
	{
		vpPosition_ = GetPredictedPosition(deltaTime);
	}

private:
	SDL_FPoint ClampToBounds(SDL_FPoint pos) const 
	{
		return {
			std::clamp(pos.x, bounds_.min.x + vpDimensions_.w / 2.0f, 
					   bounds_.max.x - vpDimensions_.w / 2.0f),
			std::clamp(pos.y, bounds_.min.y + vpDimensions_.h / 2.0f, 
					   bounds_.max.y - vpDimensions_.h / 2.0f)
		};
	}

	SDL_FPoint vpPosition_ = { 0.0f, 0.0f };
	Dimensions<float> vpDimensions_ = { 0.0f, 0.0f };

	Range<SDL_FPoint> bounds_ = { .min = { -INFINITY, -INFINITY },
								  .max = { INFINITY, INFINITY } };
	FollowTarget followTarget_;
};

class Projection
{
public:
	template <SDLPointType T>
	static T WorldToScreen(T worldP, const Camera& camera)
	{
		using ValueType = std::remove_cvref_t<decltype(T::x)>;

		auto [camX, camY] = camera.GetPosition();
		return {
			worldP.x - static_cast<ValueType>(camX),
			worldP.y - static_cast<ValueType>(camY)
		};
	}

	template <SDLRectType T>
	static T WorldToScreen(T worldR, const Camera& camera)
	{
		using ValueType = std::remove_cvref_t<decltype(T::x)>;

		auto vp = camera.GetViewport();
		return {
			worldR.x - static_cast<ValueType>(vp.x),
			worldR.y - static_cast<ValueType>(vp.y),
			worldR.w,
			worldR.h
		};
	}

	template <SDLPointType T>
	static T ScreenToWorld(T screenP, const Camera& camera)
	{
		using ValueType = std::remove_cvref_t<decltype(T::x)>;

		auto [camX, camY] = camera.GetPosition();
		return { 
			screenP.x + static_cast<ValueType>(camX), 
			screenP.y + static_cast<ValueType>(camY)
		};
	}

	template <SDLRectType T>
	static T ScreenToWorld(T screenR, const Camera& camera)
	{
		using ValueType = std::remove_cvref_t<decltype(T::x)>;

		auto vp = camera.GetViewport();
		return {
			screenR.x + static_cast<ValueType>(vp.x),
			screenR.y + static_cast<ValueType>(vp.y),
			screenR.w,
			screenR.h
		};
	}
};



//return {
//	static_cast<int>(vpPosition_.x - (vpDimensions_.w * 0.5f) +
//					 ((vpPosition_.x - (vpDimensions_.w * 0.5f)) >= 0 ? 0.5f : -0.5f)),
//	static_cast<int>(vpPosition_.y - (vpDimensions_.h * 0.5f) +
//					 ((vpPosition_.y - (vpDimensions_.h * 0.5f)) >= 0 ? 0.5f : -0.5f)),
//	static_cast<int>(vpDimensions_.w + (vpDimensions_.w >= 0 ? 0.5f : -0.5f)),
//	static_cast<int>(vpDimensions_.h + (vpDimensions_.h >= 0 ? 0.5f : -0.5f))
//};