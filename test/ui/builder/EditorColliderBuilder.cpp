#include "EditorColliderBuilder.h"

#if IMGUI_ENABLED
#include "../gui_edit/GuiEditPhysics.h"
#include "../gui_edit/GuiEditPropertyTable.h"
#include "../gui_edit/GuiEditIncludes.h"
#include "../../../components/util/ComponentValidPreds.h"

namespace ui {

namespace {

void DrawHullVector(std::optional<std::vector<SDL_FPoint>>& opVec)
{
	PropertyGroup("Hull", [&] {
		return Property("", opVec, VecArgs{ .minSize = 3 });
	});
}

} // unnamed

bool EditorColliderBuilder::Draw(Entity& e, SceneFixture& fixture)
{
	if (!e.HasComponent<RigidBody>(&RigidBodyValid))
	{
		return false;
	}

	auto& body = e.GetComponent<RigidBody>().body;
	const auto bodyPos = body.GetData().GetPosition();

	colliderEditUtility_.Draw(shapeParams_, bodyPos, fixture.GetCamera());

	if (!BeginPropertyTable())
	{
		return false;
	}

	PropertyGroup("Shape Parameters", [&] {
		ImGui::BeginDisabled();

		Property("Shape Type", shapeParams_.shapeType);
		Property("Dimensions", shapeParams_.dimensions);
		DrawHullVector(shapeParams_.hull);
		Property("Radius", shapeParams_.radius);
		Property("Local Position", shapeParams_.localPosition);
		Property("Local Rotation", shapeParams_.localRotation);

		ImGui::EndDisabled();

		return PropertyEditState::None;
	}, { .flags = ImGuiTreeNodeFlags_DefaultOpen });

	PropertyGroup("Collider Settings", [&] {
		Property("Density", colliderSettings_.density);
		Property("Friction", colliderSettings_.friction);
		Property("Restitution", colliderSettings_.restitution);

		PropertyGroup("Enable Events", [&] {
			Property("Contact", colliderSettings_.enableEvents.contact);
			Property("Hit", colliderSettings_.enableEvents.hit);
			Property("Sensor", colliderSettings_.enableEvents.sensor);

			return PropertyEditState::None;
		});

		Property("Enable Collision", colliderSettings_.enableCollision);
		Property("Is Sensor", colliderSettings_.isSensor);

		return PropertyEditState::None;
	}, { .flags = ImGuiTreeNodeFlags_DefaultOpen });

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	const bool canBuild = colliderEditUtility_.CanBuild();

	ImGui::BeginDisabled(!canBuild);

	bool built = false;

	if (ImGui::Button("Done"))
	{
		e.RemoveComponent<Collider>();

		auto& col = e.AddComponent(ComponentBuilder<Collider>{}
		.WithShapeParameters(shapeParams_)
		.WithColliderSettings(colliderSettings_)
		.Build(body));

		assert(col.shape.GetData().IsValid());

		built = true;
	}

	ImGui::EndDisabled();

	EndPropertyTable();

	return built;
}

void EditorColliderBuilder::SetIsActiveImpl(bool val)
{
	if (IsActive() && !val)
	{
		shapeParams_ = {};
		colliderSettings_ = {};
		colliderEditUtility_.Reset();
	}
}

} // ui

#endif