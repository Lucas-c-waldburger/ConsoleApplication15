#pragma once
#include "ComponentBuilder.h"
#include "../CameraTargetComponent.h"
#include "../../systems/CameraSystem.h"

template<>
class ComponentBuilder<CameraTarget>
{
public:
	ComponentBuilder() = default;

	ComponentBuilder& WithOffset(SDL_FPoint offset)
	{
		component_.offset = offset;
		return *this;
	}
	ComponentBuilder& WithFollowSpeed(float followSpeed)
	{
		component_.followSpeed = followSpeed;
		return *this;
	}

	CameraTarget Build(CameraSystem& cameraSystem)
	{
		return component_;
	}

private:
	CameraTarget component_;
};