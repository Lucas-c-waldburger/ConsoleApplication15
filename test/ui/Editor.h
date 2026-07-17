#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../Fixtures.h"
#include "../../core/commonObjects.h"
#include "EntityDragUtility.h"
#include "CameraControlUtility.h"
//#include "InspectorComponentPanel.h"

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

	struct UpdateState
	{
		PanelType forcePanelOpen = PanelType::None;
		Entity_t forceEntitySelectionForEdit = kInvalidEntity;

		int GetTabFlags(PanelType test) const
		{
			int flags = 0;
			if (forcePanelOpen == test)
			{
				flags |= ImGuiTabItemFlags_SetSelected;
			}
			return flags;
		}

		UpdateState Take()
		{
			UpdateState old = *this;
			forcePanelOpen = PanelType::None;
			forceEntitySelectionForEdit = kInvalidEntity;
			
			return old;
		}
	};

	static Result<Void> Init(SceneFixture::SharedPtr& scene);

	static void Update(ResourceContext& ctx, float dt);

	static PanelType GetActivePanel() { return activePanel_; }

	static void SetActivePanel(PanelType panelType) { activePanel_ = panelType; }

	static const UpdateState& GetUpdateState() { return updateState_; }

private:
	Editor() = default;

	static void HandleEntityDrag(const ResourceContext& ctx, Entity_t selectedEntityAtUpdateStart);
	static void HandleCameraControl(ResourceContext& ctx, float dt);

	static void UpdateForHistoryChange();
	
	static inline PanelType activePanel_ = PanelType::Entities;
	static inline EntityDragUtility entityDrag_{};
	static inline CameraControlUtility cameraControl_{};
	static inline UpdateState updateState_{};

};

} // ui

#endif