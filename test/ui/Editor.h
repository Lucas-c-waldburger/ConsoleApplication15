#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../Fixtures.h"
#include "../../core/commonObjects.h"
#include "EntityDragUtility.h"
#include "CameraControlUtility.h"

class Camera;
class TextureRepository;
class SystemManager;
class B2World;

namespace ui {

class Editor
{
public:
	enum class PanelType
	{
		None,
		Entities,
		Systems,
		Components 
	};

	struct ResourceContext
	{
		Camera& camera;
		TextureRepository& textureRepo;
		B2World& world;
		SystemManager& systemManager;

		static ResourceContext Create(SceneFixture::SharedPtr& scene);
	};

	static Result<Void> Init(SceneFixture::SharedPtr& scene);

	static void Update(ResourceContext& ctx, float dt);

	static PanelType GetActivePanel() { return activePanel_; }

private:
	Editor() = default;

	static void HandleEntityDrag(const ResourceContext& ctx, Entity_t selectedEntityAtUpdateStart);
	static void HandleCameraControl(ResourceContext& ctx, float dt);
	
	static inline PanelType activePanel_ = PanelType::Entities;
	static inline EntityDragUtility entityDrag_{};
	static inline CameraControlUtility cameraControl_{};
};

} // ui

#endif