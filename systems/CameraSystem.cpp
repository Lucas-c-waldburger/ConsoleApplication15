#include "CameraSystem.h"
#include "../ecs/Ecs.h"

namespace {

constexpr bool CameraInStopRadius(SDL_FPoint a, SDL_FPoint b, float radius)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return dx * dx + dy * dy <= radius * radius;
}

constexpr SDL_FPoint CalculateLerp(SDL_FPoint a, SDL_FPoint b, float t)
{
	return {
		a.x + (b.x - a.x) * t,
		a.y + (b.y - a.y) * t
	};
}

SDL_FPoint CalculateCameraPositionFromTransform(SDL_FPoint currentCameraPos,
											    const Transform& targetTransform,
											    const CameraTarget& cameraTarget,
											    float deltaTime)
{
	SDL_FPoint desired = {
		targetTransform.position.x + cameraTarget.offset.x,
		targetTransform.position.y + cameraTarget.offset.y
	};

	if (CameraInStopRadius(currentCameraPos, desired, cameraTarget.stopRadius))
	{
		return currentCameraPos;
	}

	float t = 1.0f - std::exp(-cameraTarget.followSpeed * deltaTime); // smoothstep-like

	return CalculateLerp(currentCameraPos, desired, t);
}

} // unnamed


void CameraSystem::Update(float deltaTime)
{
	if (IsPaused())
	{
		return;
	}

	auto entities = ECS::GetAllEntitiesWith<CameraTarget, Transform>();

	if (entities.size() > 1)
	{
		LOG_WARNING("Multiple camera targets are set");
	}

	for (auto& entity : entities)
	{
		const auto& cameraTarget = entity.GetComponent<CameraTarget>();
		const auto& transform = entity.GetComponent<Transform>();

		SDL_FPoint newPos = 
			CalculateCameraPositionFromTransform(camera_.GetPosition(), transform, 
												 cameraTarget, deltaTime);

		camera_.SetPosition(newPos, false);
	}
}

void CameraSystem::SetCameraTarget(Entity& newTarget, std::optional<CameraTarget> camTargetCmp)
{
	auto entities = ECS::GetAllEntitiesWith<CameraTarget>();

	for (auto& entity : entities)
	{
		entity.RemoveComponent<CameraTarget>();
	}

	if (!newTarget.HasComponent<CameraTarget>())
	{
		auto cmp = camTargetCmp.value_or(CameraTarget{});

		newTarget.AddComponent(std::move(cmp));
	}
}