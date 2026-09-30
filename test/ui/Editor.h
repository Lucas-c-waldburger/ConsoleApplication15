#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../Fixtures.h"
#include "../../core/commonObjects.h"
#include "EntityDragUtility.h"
#include "CameraControlUtility.h"
#include "asset_viewer/AssetViewer.h"
#include "WindowDocker.h"
#include "lua_text_editor/LuaTextEditor.h"

class Camera;
class TextureRepository;
class SystemManager;
class B2World;
class EventBus;

namespace ui {

struct AtUpdateBegin;

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

	using WindowType = EditorWindowType;

	struct Forcing
	{
		EditorWindowType forceWindowOpen = static_cast<EditorWindowType>(0);
		Entity_t forceEntitySelectionForEdit = kInvalidEntity;

		void Clear() { *this = Forcing{}; }
	};

	enum class ToolbarResponse
	{
		None,
		ResetForNewScene
	};

	static Result<Void> Init(SceneFixture::SharedPtr& fixture);

	static void Update(SceneFixture::WeakPtr weakScene, float dt);

	static PanelType GetActivePanel() { return activePanel_; }

	static uint8_t GetActiveWindows() { return activeWindows_; }

	static void SetActivePanel(PanelType panelType) { activePanel_ = panelType; }

	static void TearDown();

	static SceneFixture::SceneConfiguration GetSceneConfiguration();

private:
	Editor() = default;

	static void DrawGameWindow(SceneFixture& fixture);
	static void DrawEntityWindow(SceneFixture& fixture, const AtUpdateBegin& atUpdateBegin);
	static void DrawComponentWindow(SceneFixture& fixture, const AtUpdateBegin& atUpdateBegin);
	static void DrawSystemWindow(SceneFixture& fixture);
	static void DrawEventWindow(SceneFixture& fixture);
	static void DrawConsoleWindow();
	static void DrawAssetWindow(SceneFixture& fixture);

	static void DestroyEditorEntities();

	static Result<Void> ResetForNewScene(SceneFixture& scene);

	static void HandleGameWindowUserInteractions();
	static void HandleEntityDrag(const Camera& cam, Entity_t selectedEntityAtUpdateStart);
	static void HandleCameraControl(Camera& cam, float dt);

	static void DrawToolbar(SceneFixture& fixture);
	static void DrawFileMenu(SceneFixture& fixture);
	static void DrawDebugMenu(SceneFixture& fixture);

	static void UpdateForHistoryChange();

	static constexpr bool IsWindowOpen(EditorWindowType windowType) noexcept;
	static void DockspaceOverViewport();

	static bool ShouldForceEntitySelection();

	static Result<Void> InitUtilities(SceneFixture& fixture);
	static Result<Void> InitWindows(SceneFixture& fixture);
	static Result<Void> InitPanels(SceneFixture& fixture);
	
	static inline PanelType activePanel_ = PanelType::Entities;
	static inline uint8_t activeWindows_ = WindowType::GameWindow;
	static inline EntityDragUtility entityDrag_{};
	static inline CameraControlUtility cameraControl_{};
	static inline AssetViewer assetViewer_;
	static inline Forcing forcing_;
	static inline WindowDocker2 windowDocker_{};
	static inline bool needLayoutDockspace_ = true;
	static inline LuaTextEditor luaTextEditor_{};
};

} // ui

#endif