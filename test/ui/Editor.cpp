#include "Editor.h"

#if IMGUI_ENABLED
#include "GuiMouse.h"
#include "GuiResource.h"
#include "InspectorCommon.h"
#include "InspectorEntityPanel.h"
#include "InspectorSystemPanel.h"
#include "InspectorComponentPanel.h"
#include "ComponentEditHistory.h"
#include "SaveSceneUtility.h"

namespace ui {

namespace {

bool RightClickedWithComponentPanelOpen()
{
	return Editor::GetActivePanel() == Editor::PanelType::Components &&
		  GuiMouse::IsRightClicked();
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
	if (InspectorEntityPanel::GetSelection().IsEditing())
	{
		if (InspectorEntityPanel::GetSelection().entityId != selectedEntityAtUpdateStart)
		{
			entityDrag_.Reset();
		}

		auto e = ECS::GetEntityByID(InspectorEntityPanel::GetSelection().entityId);
		assert(e.IsValid());

		entityDrag_.Update(e, cam);
	}
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

void Editor::DrawToolbar(SceneFixture& fixture)
{
	assert(fixture.IsSystemRegistered<AudioSystem>());
	assert(fixture.IsSystemRegistered<SDLInputSystem>());

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::Selectable("Open"))
			{
				auto selectedFile = SceneSaveUtility::QuerySceneOpen();
				if (selectedFile.has_value())
				{
					const auto currentFile = SceneSaveUtility::GetCurrentSceneFilepath();

					auto openScene = [&fixture](const auto& path) {
						DestroyEditorEntities();

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
			}

			if (ImGui::Selectable("Save"))
			{
				LOG_IF_ERROR(SceneSaveUtility::HandleSceneSave(fixture));
			}

			if (ImGui::Selectable("Save As..."))
			{
				LOG_IF_ERROR(SceneSaveUtility::HandleSceneSaveAs(fixture));
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
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

void Editor::Update(SceneFixture::WeakPtr weakScene, float dt)
{
	auto scene = weakScene.lock();
	if (!scene)
	{
		return;
	}

	ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_MenuBar);

	DrawToolbar(*scene);

	InspectorEntityPanel::UpdateSelectionBoxPositions(scene->GetCamera());

	AtUpdateBegin atUpdateBegin{};
	auto currentState = updateState_.Take();

	if (RightClickedWithComponentPanelOpen())
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
				.camera = scene->GetCamera(),
				.textureRepo = scene->GetTextureRepository()
			};
			InspectorEntityPanel::Update(entityCtx);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Systems", nullptr, currentState.GetTabFlags(PanelType::Systems)))
		{
			activePanel_ = PanelType::Systems;

			auto sysCtx = InspectorSystemPanel::ResourceContext{ 
				.systemManager = scene->GetSystemManager(),
				.textureRepo = scene->GetTextureRepository()
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
				auto e = ECS::GetEntityByID(InspectorEntityPanel::GetSelection().entityId);
				assert(e.IsValid());

				auto cmpCtx = InspectorComponentPanel::ResourceContext{ 
					.entity = e,
					.textureRepo = scene->GetTextureRepository(),
					.world = scene->GetWorld()
				};
				InspectorComponentPanel::Update(cmpCtx);
			}

			ImGui::EndTabItem();
		}

		ImGui::EndDisabled();

		if (atUpdateBegin.historyCursor != ComponentEditHistory::GetCursor())
		{
			UpdateForHistoryChange();
		}

		ImGui::EndTabBar();
	}

	HandleEntityDrag(scene->GetCamera(), atUpdateBegin.selectedEntity);
	HandleCameraControl(scene->GetCamera(), dt);

	ImGui::End();
}

Result<Void> Editor::Init(SceneFixture::SharedPtr& scene)
{
	assert(scene);

	const bool registered = ECS::RegisterComponent<InspectorTag>();
	assert(registered);

	GuiResource::Init();
	GuiMouse::Init();

	AssignGuiStyles();

	TRY(InspectorEntityPanel::Init(*scene));
	TRY(InspectorSystemPanel::Init(*scene));
	TRY(InspectorComponentPanel::Init(*scene));

	TRY(cameraControl_.Init());

	assert(scene->IsSystemRegistered<GuiSystem>());
	auto& guiSys = scene->GetSystem<GuiSystem>();

	guiSys.SetUI([weakScene = std::weak_ptr{scene}](float dt) mutable { 
		Update(weakScene, dt); 
	});

	return kVoid;
}

} // ui

#endif