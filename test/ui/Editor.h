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
class EventBus;

namespace ui {

class Editor
{
public:
	enum class PanelType
	{
		None,
		Entities,
		Systems,
		Components,
		Events
	};

	enum WindowType : uint8_t
	{
		Main = 1 << 0,
		Console = 1 << 1
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

	enum class ToolbarResponse
	{
		None,
		ResetForNewScene
	};

	static Result<Void> Init(SceneFixture::SharedPtr& scene);

	static void Update(SceneFixture::WeakPtr weakScene, float dt);

	static PanelType GetActivePanel() { return activePanel_; }

	static void SetActivePanel(PanelType panelType) { activePanel_ = panelType; }

	static const UpdateState& GetUpdateState() { return updateState_; }

	static void TearDown();

	static SceneFixture::SceneConfiguration GetSceneConfiguration();

private:
	Editor() = default;

	static void DestroyEditorEntities();

	static Result<Void> ResetForNewScene(SceneFixture& scene);

	static void HandleEntityDrag(const Camera& cam, Entity_t selectedEntityAtUpdateStart);
	static void HandleCameraControl(Camera& cam, float dt);

	static void DrawToolbar(SceneFixture& fixture);
	static void DrawFileMenu(SceneFixture& fixture);
	static void DrawDebugMenu(SceneFixture& fixture);

	static void UpdateForHistoryChange();
	
	static inline PanelType activePanel_ = PanelType::Entities;
	static inline uint8_t activeWindows_ = WindowType::Main;
	static inline EntityDragUtility entityDrag_{};
	static inline CameraControlUtility cameraControl_{};
	static inline UpdateState updateState_{};
};

} // ui

#endif