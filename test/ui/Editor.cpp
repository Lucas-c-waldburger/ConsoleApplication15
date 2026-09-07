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
	if (Editor::GetActivePanel() != Editor::PanelType::Entities &&
		Editor::GetActivePanel() != Editor::PanelType::Components)
	{
		const auto& sel = InspectorEntityPanel::GetSelection();
		if (sel.IsHovering())
		{
			InspectorEntityPanel::ClearSelection();
		}
	}	
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

void Editor::DrawDebugMenu(SceneFixture& fixture)
{
	if (ImGui::Selectable("Console", (activeWindows_ & WindowType::Console)))
	{
		activeWindows_ ^= WindowType::Console;
	}
}

void Editor::DrawToolbar(SceneFixture& fixture)
{
	assert(fixture.IsSystemRegistered<AudioSystem>());
	assert(fixture.IsSystemRegistered<SDLInputSystem>());

	if (!ImGui::BeginMainMenuBar())
	{
		return;
	}

	if (ImGui::BeginMenu("File"))
	{
		DrawFileMenu(fixture);

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Debug"))
	{
		DrawDebugMenu(fixture);

		ImGui::EndMenu();
	}

	if (ImGui::MenuItem("Asset Viewer", nullptr, (activeWindows_ & WindowType::Assets)))
	{
		activeWindows_ ^= WindowType::Assets;
	}

	ImGui::EndMainMenuBar();
}

Result<Void> Editor::ResetForNewScene(SceneFixture& fixture)
{
	entityDrag_.Reset();
	cameraControl_.Reset();

	GuiMouse::Init();

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
};

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

	ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_MenuBar);

	DrawToolbar(*fixture);

	InspectorEntityPanel::UpdateSelectionBoxPositions(fixture->GetCamera());

	AtUpdateBegin atUpdateBegin{};
	auto currentState = updateState_.Take();

	if (UserWantsClearSelectedEntity())
	{
		InspectorEntityPanel::ClearSelection();
		InspectorComponentPanel::ClearState();

		currentState.forcePanelOpen = PanelType::Entities;
	}
	else if (currentState.forceEntitySelectionForEdit != kInvalidEntity)
	{
		InspectorEntityPanel::SetSelectedEntityForEdit(currentState.forceEntitySelectionForEdit);
		currentState.forcePanelOpen = PanelType::Components;
	}

	if (ImGui::BeginTabBar("Tabs"))
	{
		if (ImGui::BeginTabItem("Entities", nullptr, currentState.GetTabFlags(PanelType::Entities)))
		{
			activePanel_ = PanelType::Entities;

			auto entityCtx = InspectorEntityPanel::ResourceContext{ 
				.camera = fixture->GetCamera(),
				.textureRepo = *auxRepo
			};
			InspectorEntityPanel::Update(entityCtx);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Systems", nullptr, currentState.GetTabFlags(PanelType::Systems)))
		{
			activePanel_ = PanelType::Systems;

			auto sysCtx = InspectorSystemPanel::ResourceContext{ 
				.systemManager = fixture->GetSystemManager(),
				.textureRepo = *auxRepo
			};
			InspectorSystemPanel::Update(sysCtx);

			ImGui::EndTabItem();
		}

		const bool entitySelected = InspectorEntityPanel::GetSelection().IsEditing();

		if (atUpdateBegin.selectedEntity == kInvalidEntity && entitySelected)
		{
			currentState.forcePanelOpen = PanelType::Components;
		}

		ImGui::BeginDisabled(!entitySelected);
		
		if (ImGui::BeginTabItem("Components", nullptr, currentState.GetTabFlags(PanelType::Components)))
		{
			activePanel_ = PanelType::Components;

			if (entitySelected)
			{
				const auto selectedEntityId = InspectorEntityPanel::GetSelection().entityId;
				if (selectedEntityId != atUpdateBegin.selectedEntity)
				{
					InspectorComponentPanel::SetActiveBuilderType(kInvalidComponentBuilderType);
				}

				auto e = ECS::GetEntityByID(selectedEntityId);
				assert(e.IsValid());

				InspectorComponentPanel::Update(e, *fixture);
			}

			ImGui::EndTabItem();
		}

		ImGui::EndDisabled();

		if (ImGui::BeginTabItem("Events", nullptr, currentState.GetTabFlags(PanelType::Events)))
		{
			activePanel_ = PanelType::Events;

			auto evCtx = InspectorEventPanel::ResourceContext{
				.eventBus = fixture->GetEventBus(),
				.textureRepo = *auxRepo
			};
			InspectorEventPanel::Update(evCtx);

			ImGui::EndTabItem();
		}

		if (atUpdateBegin.historyCursor != ComponentEditHistory::GetCursor())
		{
			UpdateForHistoryChange();
		}

		ImGui::EndTabBar();
	}

	HandleEntityDrag(fixture->GetCamera(), atUpdateBegin.selectedEntity);
	HandleCameraControl(fixture->GetCamera(), dt);
	ClearHoverSelectionIfNeeded();

	ImGui::End();

	if (activeWindows_ & WindowType::Console)
	{
		if (!GuiConsole::Draw())
		{
			activeWindows_ &= ~WindowType::Console;
		}
	}
	if (activeWindows_ & WindowType::Assets)
	{
		if (!assetViewer_.Draw(*auxRepo))
		{
			activeWindows_ &= ~WindowType::Assets;
		}
	}
}

Result<Void> Editor::Init(SceneFixture::SharedPtr& fixture)
{
	assert(fixture);

	ECS::RegisterComponent<InspectorTag>();
	ECS::RegisterComponent<CallbackInfo>();

	GuiResource::Init();
	GuiMouse::Init();
	GuiConsole::Init();

	AssignGuiStyles();

	TRY(InspectorEntityPanel::Init(*fixture));
	TRY(InspectorSystemPanel::Init(*fixture));
	TRY(InspectorComponentPanel::Init(*fixture));
	TRY(InspectorEventPanel::Init(*fixture));

	TRY(cameraControl_.Init());
	TRY(assetViewer_.Init(*fixture));

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
}

SceneFixture::SceneConfiguration Editor::GetSceneConfiguration()
{
	static constexpr auto omitEntityDestruction = +[](const Entity& e) {
		return e.HasComponent<InspectorTag>();
	};

	return SceneFixture::SceneConfiguration{
		.flags = SceneFixture::SceneConfiguration::InitAuxTextureRepo,
		.omitEntityDestruction = omitEntityDestruction
	};
}

} // ui

#endif