#include "Editor.h"

#if IMGUI_ENABLED
#include "GuiMouse.h"
#include "GuiResource.h"
#include "InspectorCommon.h"
#include "InspectorEntityPanel.h"
#include "InspectorSystemPanel.h"
#include "InspectorComponentPanel.h"
#include "ComponentEditHistory.h"

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

auto Editor::ResourceContext::Create(SceneFixture::SharedPtr& scene) -> ResourceContext
{
	return {
		.camera = scene->GetCamera(),
		.textureRepo = scene->GetTextureRepository(),
		.world = scene->GetWorld(),
		.systemManager = scene->GetSystemManager()
	};
}

void Editor::HandleEntityDrag(const ResourceContext& ctx, Entity_t selectedEntityAtUpdateStart)
{
	if (InspectorEntityPanel::GetSelection().IsEditing())
	{
		if (InspectorEntityPanel::GetSelection().entityId != selectedEntityAtUpdateStart)
		{
			entityDrag_.Reset();
		}

		auto e = ECS::GetEntityByID(InspectorEntityPanel::GetSelection().entityId);
		assert(e.IsValid());

		entityDrag_.Update(e, ctx.camera);
	}
}

void Editor::HandleCameraControl(ResourceContext& ctx, float dt)
{
	if (entityDrag_.IsDragging())
	{
		cameraControl_.Reset();
	}
	else
	{
		cameraControl_.UpdateScroll(ctx.camera, dt);
	}


	cameraControl_.UpdateZoom(ctx.camera);
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

void Editor::Update(ResourceContext& ctx, float dt)
{
	ImGui::Begin("Editor");

	InspectorEntityPanel::UpdateSelectionBoxPositions(ctx.camera);

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

			auto entityCtx = InspectorEntityPanel::ResourceContext{ .camera = ctx.camera,
																	.textureRepo = ctx.textureRepo };
			InspectorEntityPanel::Update(entityCtx);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Systems", nullptr, currentState.GetTabFlags(PanelType::Systems)))
		{
			activePanel_ = PanelType::Systems;

			auto sysCtx = InspectorSystemPanel::ResourceContext{ .systemManager = ctx.systemManager,
																 .textureRepo = ctx.textureRepo };
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

				auto cmpCtx = InspectorComponentPanel::ResourceContext{ .entity = e,
																		.textureRepo = ctx.textureRepo,
																		.world = ctx.world };
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

	HandleEntityDrag(ctx, atUpdateBegin.selectedEntity);
	HandleCameraControl(ctx, dt);

	ImGui::End();
}

Result<Void> Editor::Init(SceneFixture::SharedPtr& scene)
{
	const bool registered = ECS::RegisterComponent<InspectorTag>();
	assert(registered);

	GuiResource::Init();
	GuiMouse::Init();

	AssignGuiStyles();

	TRY(InspectorEntityPanel::Init(scene));
	TRY(InspectorSystemPanel::Init(scene));
	TRY(InspectorComponentPanel::Init(scene));

	TRY(cameraControl_.Init());

	assert(scene->IsSystemRegistered<GuiSystem>());
	auto& guiSys = scene->GetSystem<GuiSystem>();

	guiSys.SetUI([ctx = ResourceContext::Create(scene)](float dt) mutable { Update(ctx, dt); });

	return kVoid;
}

} // ui

#endif