#include "Editor.h"

#if IMGUI_ENABLED
#include "GuiMouse.h"
#include "GuiResource.h"
#include "InspectorCommon.h"
#include "InspectorEntityPanel.h"
#include "InspectorSystemPanel.h"
#include "InspectorComponentPanel.h"

namespace ui {

namespace {

bool RightClickedWithComponentPanelOpen()
{
	return Editor::GetActivePanel() == Editor::PanelType::Components &&
		  GuiMouse::IsRightClicked();
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

struct State
{
	Editor::PanelType startOpen = Editor::PanelType::None;
	const bool beganWithEntitySelected = InspectorEntityPanel::GetSelection().IsEditing();

	int GetTabFlags(const Editor::PanelType currentPanel) const
	{
		int flags = 0;
		if (startOpen == currentPanel)
		{
			flags |= ImGuiTabItemFlags_SetSelected;
		}
		return flags;
	};
};

void Editor::Update(ResourceContext& ctx, float dt)
{
	ImGui::Begin("Editor");

	State state{};

	InspectorEntityPanel::UpdateSelectionBoxPositions(ctx.camera);

	if (RightClickedWithComponentPanelOpen())
	{
		InspectorEntityPanel::ClearSelection();
		InspectorComponentPanel::ClearState();

		state.startOpen = PanelType::Entities;
	}

	Entity_t selectedEntityAtStart = InspectorEntityPanel::GetSelection().IsEditing()
		? InspectorEntityPanel::GetSelection().entityId
		: kInvalidEntity;

	if (ImGui::BeginTabBar("Tabs"))
	{
		if (ImGui::BeginTabItem("Entities", nullptr, state.GetTabFlags(PanelType::Entities)))
		{
			if (activePanel_ == PanelType::Components)
			{
				//InspectorEntityPanel::ClearSelection();
			}

			activePanel_ = PanelType::Entities;

			auto entityCtx = InspectorEntityPanel::ResourceContext{ .camera = ctx.camera,
																	.textureRepo = ctx.textureRepo };
			InspectorEntityPanel::Update(entityCtx);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Systems", nullptr, state.GetTabFlags(PanelType::Systems)))
		{
			activePanel_ = PanelType::Systems;

			auto sysCtx = InspectorSystemPanel::ResourceContext{ .systemManager = ctx.systemManager,
																 .textureRepo = ctx.textureRepo };
			InspectorSystemPanel::Update(sysCtx);

			ImGui::EndTabItem();
		}

		const bool entitySelected = InspectorEntityPanel::GetSelection().IsEditing();

		if (!state.beganWithEntitySelected && entitySelected)
		{
			state.startOpen = PanelType::Components;
		}

		ImGui::BeginDisabled(!entitySelected);
		
		if (ImGui::BeginTabItem("Components", nullptr, state.GetTabFlags(PanelType::Components)))
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

		ImGui::EndTabBar();
	}

	HandleEntityDrag(ctx, selectedEntityAtStart);
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