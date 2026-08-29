#include "EditorRigidBodyBuilder.h"

#if IMGUI_ENABLED
#include "../gui_edit/GuiEditPhysics.h"
#include "../gui_edit/GuiEditPropertyTable.h"
#include "../gui_edit/GuiEditIncludes.h"

namespace ui {

bool EditorRigidBodyBuilder::Draw(Entity& e, SceneFixture& fixture)
{
	assert(e.HasComponent<Transform>());

	SDL_FPoint pos = bodyParams_.position;
	if (!manuallySelectingPosition_)
	{
		pos = e.GetComponent<Transform>().position;
		bodyParams_.position = pos;
	}

	if (!BeginPropertyTable())
	{
		return false;
	}

	PropertyGroup("Body Parameters", [&] {

		Property("Body Type", bodyParams_.bodyType);

		if (Property("Position", pos))
		{
			manuallySelectingPosition_ = true;
			bodyParams_.position = pos;
		}

		Property("Gravity Scale", bodyParams_.gravityScale, DragArgs<float>{ 0.05, 0.0f, 100.0f });
		Property("Fixed Rotation", bodyParams_.fixedRotation);

		return PropertyEditState::None;
	}, { .flags = ImGuiTreeNodeFlags_DefaultOpen });

	PropertyGroup("Body Limits", [&] {
		return Property("", bodyLimits_, ImGuiTreeNodeFlags_DefaultOpen);
	}, { .flags = ImGuiTreeNodeFlags_DefaultOpen });

	bool built = false;

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	if (ImGui::Button("Done"))
	{
		e.RemoveComponent<RigidBody>();

		auto& rb = e.AddComponent(ComponentBuilder<RigidBody>{}
		.WithBodyParameters(bodyParams_)
		.WithBodyLimits(bodyLimits_)
		.Build(fixture.GetWorld()));

		assert(rb.body.GetData().IsValid());

		built = true;
	}

	EndPropertyTable();

	return built;
}

void EditorRigidBodyBuilder::SetIsActiveImpl(bool val)
{
	if (IsActive() && !val)
	{
		bodyParams_ = {};
		bodyLimits_ = {};
		manuallySelectingPosition_ = false;
	}
}




} // ui

#endif