#include "Editor.h"

#if IMGUI_ENABLED
#include "GuiMouse.h"
#include "GuiResource.h"
#include "InspectorCommon.h"
#include "InspectorEntityPanel.h"
#include "InspectorSystemPanel.h"
#include "InspectorComponentPanel.h"
#include "InspectorEventPanel.h"
#include "ComponentEditHistory.h"
#include "SaveSceneUtility.h"
#include "GuiConsole.h"
#include "../../render/AspectRatioFit.h"

namespace ui {

namespace {

bool UserWantsClearSelectedEntity()
{
	return Editor::GetActivePanel() == Editor::PanelType::Components &&
		   ImGui::IsKeyPressed(ImGuiKey_Escape);
}

void DrawSelectionState()
{
	const auto& sel = InspectorEntityPanel::GetSelection();
	std::string name = "kInvalidEntity";
	if (auto e = ECS::GetEntityByID(sel.entityId); e.IsValid())
	{
		assert(e.HasComponent<Name>());
		name = e.GetComponent<Name>();
		while (name.size() < 14)
		{
			name += " ";
		}
	}

	std::string selType = sel.selectionType == InspectorEntityPanel::SelectionType::Edit ?
		"Edit " : sel.selectionType == InspectorEntityPanel::SelectionType::Hover ? "Hover" : "None ";

	std::string selReport = std::format("{} : {}", name, selType);

	ImGui::TextUnformatted(selReport.c_str());
}

void ClearHoverSelectionIfNeeded()
{
	if ((Editor::GetActiveWindows() & (Editor::WindowType::EntityWindow | 
									   Editor::WindowType::ComponentWindow)) == 0)
	{
		const auto& sel = InspectorEntityPanel::GetSelection();
		if (sel.IsHovering())
		{
			InspectorEntityPanel::ClearSelection();
		}
	}	
}

void DrawGameDisplayWindow(SceneFixture& fixture)
{
	const auto& renderTarget = fixture.GetRenderTarget();
	const auto& displayArea = fixture.GetGameDisplayArea();

	const auto toolbarHeight = ImGui::GetFrameHeight();

	ImGui::SetNextWindowSize(ImVec2(displayArea.w, displayArea.h), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(0.0f, toolbarHeight), ImGuiCond_FirstUseEver);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGui::Begin("Game Window", nullptr, (ImGuiWindowFlags_NoTitleBar |
										  ImGuiWindowFlags_NoBackground));

	const ImVec2 pos = ImGui::GetCursorScreenPos();
	const ImVec2 size = ImGui::GetContentRegionAvail();

	const SDL_FRect contentRect{
		pos.x,
		pos.y,
		size.x,
		size.y
	};

	fixture.SetGameDisplayArea(contentRect);

	auto tx = GuiTextureConverter::FromRenderTarget(renderTarget);

	const ImVec2 available = ImGui::GetContentRegionAvail();

	const float scale = GetAspectRatioFitScale(
		AspectRatioFit::Letterbox,
		available.x,
		available.y,
		static_cast<float>(renderTarget.width),
		static_cast<float>(renderTarget.height)
	);

	const ImVec2 displaySize{
		renderTarget.width * scale,
		renderTarget.height * scale
	};

	ImGui::Image(
		tx.textureId,
		displaySize,
		tx.uv0,
		tx.uv1);

	ImGui::End();

	ImGui::PopStyleVar(3);
}

} // unnamed

void Editor::DestroyEditorEntities()
{
	auto es = ECS::GetAllActiveEntities();
	for (auto& e : es)
	{
		if (e.HasComponent<InspectorTag>())
		{
			e.Destroy();
		}
	}
}

void Editor::HandleEntityDrag(const Camera& cam, Entity_t selectedEntityAtUpdateStart)
{
	if (InspectorComponentPanel::GetActiveBuilderType() == ComponentBuilderType::Collider)
	{
		return;
	}

	if (!InspectorEntityPanel::GetSelection().IsEditing())
	{
		return;
	}

	if (InspectorEntityPanel::GetSelection().entityId != selectedEntityAtUpdateStart)
	{
		entityDrag_.Reset();
	}

	auto e = ECS::GetEntityByID(InspectorEntityPanel::GetSelection().entityId);
	assert(e.IsValid());

	entityDrag_.Update(e, cam);
}

void Editor::HandleCameraControl(Camera& cam, float dt)
{
	if (entityDrag_.IsDragging())
	{
		cameraControl_.Reset();
	}
	else
	{
		cameraControl_.UpdateScroll(cam, dt);
	}

	cameraControl_.UpdateZoom(cam);
}

void Editor::DrawFileMenu(SceneFixture& fixture)
{
	if (!ImGui::Selectable("Open"))
	{
		return;
	}

	auto selectedFile = SceneSaveUtility::QuerySceneOpen();
	if (selectedFile.has_value())
	{
		const auto currentFile = SceneSaveUtility::GetCurrentSceneFilepath();

		auto openScene = [&fixture](const auto& path) {

			auto response = SceneSaveUtility::HandleSceneOpen(path, fixture);

			for (const auto& err : response.errors)
			{
				LOG_ERROR(err.GetMessage());
			}

			return response.success;
			};

		if (!openScene(*selectedFile))
		{
			LOG_ERROR("Could not open new scene. Attempting to reopen last scene");

			openScene(currentFile);
		}

		LOG_IF_ERROR(ResetForNewScene(fixture));
	}

	if (ImGui::Selectable("Save"))
	{
		LOG_IF_ERROR(SceneSaveUtility::HandleSceneSave(fixture));
	}

	if (ImGui::Selectable("Save As..."))
	{
		LOG_IF_ERROR(SceneSaveUtility::HandleSceneSaveAs(fixture));
	}
}

void Editor::DrawToolbar(SceneFixture& fixture)
{
	assert(fixture.IsSystemRegistered<AudioSystem2>());
	assert(fixture.IsSystemRegistered<SDLInputSystem>());

	if (!ImGui::BeginMainMenuBar())
	{
		return;
	}

	auto displayArea = fixture.GetGameDisplayArea();
	displayArea.y = ImGui::GetFrameHeight();
	fixture.SetGameDisplayArea(displayArea);

	if (ImGui::BeginMenu("File"))
	{
		DrawFileMenu(fixture);

		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Asset", nullptr, (activeWindows_ & WindowType::AssetWindow)))
	{
		activeWindows_ ^= WindowType::AssetWindow;
	}
	if (ImGui::MenuItem("Entity", nullptr, (activeWindows_ & WindowType::EntityWindow)))
	{
		activeWindows_ ^= WindowType::EntityWindow;
	}
	if (ImGui::MenuItem("Component", nullptr, (activeWindows_ & WindowType::ComponentWindow),
						InspectorEntityPanel::GetSelection().IsEditing()))
	{
		activeWindows_ ^= WindowType::ComponentWindow;
	}
	if (ImGui::MenuItem("System", nullptr, (activeWindows_ & WindowType::SystemWindow)))
	{
		activeWindows_ ^= WindowType::SystemWindow;
	}
	if (ImGui::MenuItem("Console", nullptr, (activeWindows_ & WindowType::ConsoleWindow)))
	{
		activeWindows_ ^= WindowType::ConsoleWindow;
	}

	ImGui::EndMainMenuBar();
}

Result<Void> Editor::ResetForNewScene(SceneFixture& fixture)
{
	entityDrag_.Reset();
	cameraControl_.Reset();

	const auto& renderTarget = fixture.GetRenderTarget();
	GuiMouse::Init(renderTarget.width, renderTarget.height, fixture.GetGameDisplayArea());

	ComponentEditHistory::Reset();
	TRY(InspectorComponentPanel::ResetForNewScene(fixture));
	TRY(InspectorEntityPanel::ResetForNewScene(fixture));
	TRY(InspectorSystemPanel::ResetForNewScene(fixture));
	TRY(InspectorEventPanel::ResetForNewScene(fixture));

	updateState_.forceEntitySelectionForEdit = kInvalidEntity;
	updateState_.forcePanelOpen = PanelType::Entities;

	return kVoid;
}

void Editor::UpdateForHistoryChange()
{
	const Entity_t entityAtCurrentRecord = ComponentEditHistory::GetEntityForCurrentRecord();
	if (entityAtCurrentRecord != kInvalidEntity)
	{
		const auto& selection = InspectorEntityPanel::GetSelection();
		if (selection.entityId != entityAtCurrentRecord ||
			selection.selectionType != InspectorEntityPanel::SelectionType::Edit)
		{
			updateState_.forceEntitySelectionForEdit = entityAtCurrentRecord;
			updateState_.forcePanelOpen = PanelType::Components;
		}
	}
}

struct AtUpdateBegin
{
	const Entity_t selectedEntity = InspectorEntityPanel::GetSelection().IsEditing()
		? InspectorEntityPanel::GetSelection().entityId
		: kInvalidEntity;
	const int historyCursor = ComponentEditHistory::GetCursor();
	const uint8_t activeWindows = Editor::GetActiveWindows();
};

//void Editor::Update(SceneFixture::WeakPtr weakFixture, float dt)
//{
//	auto fixture = weakFixture.lock();
//	if (!fixture)
//	{
//		LOG_ERROR("Could not lock scene fixture");
//
//		return;
//	}
//
//	const auto& auxRepo = fixture->GetAuxTextureRepository();
//	if (!auxRepo)
//	{
//		LOG_ERROR("Auxilliary Texture Repository was null");
//
//		return;
//	}
//
//	ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_MenuBar);
//
//	DrawToolbar(*fixture);
//
//	InspectorEntityPanel::UpdateSelectionBoxPositions(fixture->GetCamera());
//
//	AtUpdateBegin atUpdateBegin{};
//	auto currentState = updateState_.Take();
//
//	if (UserWantsClearSelectedEntity())
//	{
//		InspectorEntityPanel::ClearSelection();
//		InspectorComponentPanel::ClearState();
//
//		currentState.forcePanelOpen = PanelType::Entities;
//	}
//	else if (currentState.forceEntitySelectionForEdit != kInvalidEntity)
//	{
//		InspectorEntityPanel::SetSelectedEntityForEdit(currentState.forceEntitySelectionForEdit);
//		currentState.forcePanelOpen = PanelType::Components;
//	}
//
//	if (ImGui::BeginTabBar("Tabs"))
//	{
//		if (ImGui::BeginTabItem("Entities", nullptr, currentState.GetTabFlags(PanelType::Entities)))
//		{
//			activePanel_ = PanelType::Entities;
//
//			auto entityCtx = InspectorEntityPanel::ResourceContext{ 
//				.camera = fixture->GetCamera(),
//				.textureRepo = *auxRepo
//			};
//			InspectorEntityPanel::Update(entityCtx);
//
//			ImGui::EndTabItem();
//		}
//
//		if (ImGui::BeginTabItem("Systems", nullptr, currentState.GetTabFlags(PanelType::Systems)))
//		{
//			activePanel_ = PanelType::Systems;
//
//			auto sysCtx = InspectorSystemPanel::ResourceContext{ 
//				.systemManager = fixture->GetSystemManager(),
//				.textureRepo = *auxRepo
//			};
//			InspectorSystemPanel::Update(sysCtx);
//
//			ImGui::EndTabItem();
//		}
//
//		const bool entitySelected = InspectorEntityPanel::GetSelection().IsEditing();
//
//		if (atUpdateBegin.selectedEntity == kInvalidEntity && entitySelected)
//		{
//			currentState.forcePanelOpen = PanelType::Components;
//		}
//
//		ImGui::BeginDisabled(!entitySelected);
//		
//		if (ImGui::BeginTabItem("Components", nullptr, currentState.GetTabFlags(PanelType::Components)))
//		{
//			activePanel_ = PanelType::Components;
//
//			if (entitySelected)
//			{
//				const auto selectedEntityId = InspectorEntityPanel::GetSelection().entityId;
//				if (selectedEntityId != atUpdateBegin.selectedEntity)
//				{
//					InspectorComponentPanel::SetActiveBuilderType(kInvalidComponentBuilderType);
//				}
//
//				auto e = ECS::GetEntityByID(selectedEntityId);
//				assert(e.IsValid());
//
//				InspectorComponentPanel::Update(e, *fixture);
//			}
//
//			ImGui::EndTabItem();
//		}
//
//		ImGui::EndDisabled();
//
//		if (ImGui::BeginTabItem("Events", nullptr, currentState.GetTabFlags(PanelType::Events)))
//		{
//			activePanel_ = PanelType::Events;
//
//			auto evCtx = InspectorEventPanel::ResourceContext{
//				.eventBus = fixture->GetEventBus(),
//				.textureRepo = *auxRepo
//			};
//			InspectorEventPanel::Update(evCtx);
//
//			ImGui::EndTabItem();
//		}
//
//		if (atUpdateBegin.historyCursor != ComponentEditHistory::GetCursor())
//		{
//			UpdateForHistoryChange();
//		}
//
//		ImGui::EndTabBar();
//	}
//
//	HandleEntityDrag(fixture->GetCamera(), atUpdateBegin.selectedEntity);
//	HandleCameraControl(fixture->GetCamera(), dt);
//	ClearHoverSelectionIfNeeded();
//
//	ImGui::End();
//
//	if (activeWindows_ & WindowType::ConsoleWindow)
//	{
//		if (!GuiConsole::Draw())
//		{
//			activeWindows_ &= ~WindowType::ConsoleWindow;
//		}
//	}
//	if (activeWindows_ & WindowType::AssetWindow)
//	{
//		if (!assetViewer_.Draw(*fixture))
//		{
//			activeWindows_ &= ~WindowType::AssetWindow;
//		}
//	}
//}

void Editor::Update(SceneFixture::WeakPtr weakFixture, float dt)
{
	auto fixture = weakFixture.lock();
	if (!fixture)
	{
		LOG_ERROR("Could not lock scene fixture");

		return;
	}

	const auto& auxRepo = fixture->GetAuxTextureRepository();
	if (!auxRepo)
	{
		LOG_ERROR("Auxilliary Texture Repository was null");

		return;
	}

	AtUpdateBegin atUpdateBegin{};

	DrawToolbar(*fixture);

	DrawGameDisplayWindow(*fixture);

	//auto currentState = updateState_.Take();

	if (UserWantsClearSelectedEntity())
	{
		InspectorEntityPanel::ClearSelection();
		InspectorComponentPanel::ClearState();

		//currentState.forcePanelOpen = PanelType::Entities;
	}
	//else if (currentState.forceEntitySelectionForEdit != kInvalidEntity)
	//{
	//	InspectorEntityPanel::SetSelectedEntityForEdit(currentState.forceEntitySelectionForEdit);
	//	currentState.forcePanelOpen = PanelType::Components;
	//}

	InspectorEntityPanel::UpdateSelectionBoxPositions(fixture->GetCamera());

	if (activeWindows_ & WindowType::EntityWindow)
	{
		if (((atUpdateBegin.activeWindows & WindowType::EntityWindow) == 0) &&
			InspectorEntityPanel::GetSelection().IsEditing())
		{
			InspectorEntityPanel::SetSelectionBoxVisibility(true);
		}

		if (!InspectorEntityPanel::Draw(*fixture))
		{
			activeWindows_ &= ~WindowType::EntityWindow;
		}
	}
	else if (atUpdateBegin.activeWindows & WindowType::EntityWindow)
	{
		InspectorEntityPanel::SetSelectionBoxVisibility(false);
	}

	if (!InspectorEntityPanel::GetSelection().IsEditing())
	{
		activeWindows_ &= ~WindowType::ComponentWindow;
	}
	if (activeWindows_ & WindowType::ComponentWindow)
	{
		const auto selectedEntityId = InspectorEntityPanel::GetSelection().entityId;
		if (selectedEntityId != atUpdateBegin.selectedEntity)
		{
			InspectorComponentPanel::SetActiveBuilderType(kInvalidComponentBuilderType);
		}

		auto e = ECS::GetEntityByID(selectedEntityId);
		assert(e.IsValid());

		if (!InspectorComponentPanel::Draw(e, *fixture))
		{
			activeWindows_ &= ~WindowType::ComponentWindow;
		}

		if (atUpdateBegin.historyCursor != ComponentEditHistory::GetCursor())
		{
			UpdateForHistoryChange();
		}
	}

	if (activeWindows_ & WindowType::SystemWindow)
	{
		if (!InspectorSystemPanel::Draw(*fixture))
		{
			activeWindows_ &= ~WindowType::SystemWindow;
		}
	}

	if (activeWindows_ & WindowType::EventWindow)
	{
		if (!InspectorEventPanel::Draw(*fixture))
		{
			activeWindows_ &= ~WindowType::EventWindow;
		}
	}

	if (activeWindows_ & WindowType::ConsoleWindow)
	{
		if (!GuiConsole::Draw())
		{
			activeWindows_ &= ~WindowType::ConsoleWindow;
		}
	}
	if (activeWindows_ & WindowType::AssetWindow)
	{
		if (!assetViewer_.Draw(*fixture))
		{
			activeWindows_ &= ~WindowType::AssetWindow;
		}
	}

	HandleEntityDrag(fixture->GetCamera(), atUpdateBegin.selectedEntity);
	HandleCameraControl(fixture->GetCamera(), dt);
}

Result<Void> Editor::InitUtilities(SceneFixture& fixture)
{
	GuiResource::Init();

	const auto& renderTarget = fixture.GetRenderTarget();
	GuiMouse::Init(renderTarget.width, renderTarget.height, fixture.GetGameDisplayArea());

	AssignGuiStyles();

	TRY(cameraControl_.Init());

	return kVoid;
}

Result<Void> Editor::InitWindows(SceneFixture& fixture)
{
	GuiConsole::Init();
	TRY(assetViewer_.Init(fixture));

	return kVoid;
}

Result<Void> Editor::InitPanels(SceneFixture& fixture)
{
	TRY(InspectorEntityPanel::Init(fixture));
	TRY(InspectorSystemPanel::Init(fixture));
	TRY(InspectorComponentPanel::Init(fixture));
	TRY(InspectorEventPanel::Init(fixture));

	return kVoid;
}

Result<Void> Editor::Init(SceneFixture::SharedPtr& fixture)
{
	assert(fixture);

	ECS::RegisterComponent<InspectorTag>();
	ECS::RegisterComponent<CallbackInfo>();

	TRY(InitUtilities(*fixture));
	TRY(InitPanels(*fixture));
	TRY(InitWindows(*fixture));

	assert(fixture->IsSystemRegistered<GuiSystem>());
	auto& guiSys = fixture->GetSystem<GuiSystem>();

	guiSys.SetUI([weakFixture = std::weak_ptr{fixture}](float dt) mutable {
		Update(weakFixture, dt); 
	});

	return kVoid;
}

void Editor::TearDown()
{
	InspectorEventPanel::TearDown();
	assetViewer_.TearDown();
}

SceneFixture::SceneConfiguration Editor::GetSceneConfiguration()
{
	static constexpr auto omitEntityDestruction = +[](const Entity& e) {
		return e.HasComponent<InspectorTag>();
	};

	return SceneFixture::SceneConfiguration{
		.flags = SceneFixture::SceneConfiguration::InitAuxTextureRepo |
				 SceneFixture::SceneConfiguration::OverrideRenderPresent,
		.omitEntityDestruction = omitEntityDestruction,
		.displayAreaFit = AspectRatioFit::Stretch
	};
}

} // ui

#endif