#pragma once
#include "System.h"
#include "../camera/Camera.h"
#include "../components/CameraTargetComponent.h"
#include <optional>

class Entity;

class CameraSystem : public System
{
public:
	CameraSystem() = default;
	explicit CameraSystem(Camera cam) : camera_(std::move(cam)) {}
	explicit CameraSystem(Dimensions<float> vpSize) : camera_(vpSize) {}
	~CameraSystem() = default;

	void Update(float deltaTime);

	void SetCameraTarget(Entity& newTarget, std::optional<CameraTarget> camTargetCmp = {});

	//bool HasSingleTarget() const;

	Camera& GetCamera() { return camera_; }

private:
	Camera camera_;
};