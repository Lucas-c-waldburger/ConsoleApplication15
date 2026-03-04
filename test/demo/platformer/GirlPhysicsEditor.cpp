#include "GirlPhysicsEditor.h"

#if IMGUI_ENABLED

#include "../../../ecs/Ecs.h"
#include "GirlStatConfig.h"
#include "../../../systems/GuiSystem.h"

namespace test {

Result<Void> GirlPhysicsEditor::Init(GuiSystem& guiSystem, Entity& girl, bool useOriginalDefaults)
{
	assert((girl.HasComponents<AnimationDeltas, MoveTargets>()));

	if (!GirlStatConfig::ConfigFileExists() || useOriginalDefaults)
	{
		TRY(GirlStatConfig::SerializeToJson(
			&kGirlBaseAnimationDeltas,
			&kGirlBaseMoveTargets));
	}

	TRY(GirlStatConfig::DeserializeFromJson(
		&girl.GetComponent<AnimationDeltas>(),
		&girl.GetComponent<MoveTargets>()));

	guiSystem.AddWidget("Girl Physics Editor", [girl] mutable {
		if (girl.IsValid())
		{
			GirlPhysicsEditor::Draw(girl);
		}
	});

	return kVoid;
}

void GirlPhysicsEditor::Draw(Entity& girl)
{
	if (ImGui::Button("Animations"))
	{
		ImGui::OpenPopup("AnimationsPopup");
	}
	if (ImGui::BeginPopup("AnimationsPopup"))
	{
		DrawAnimationsPopup(girl);

		ImGui::EndPopup();
	}

	if (ImGui::Button("Physics"))
	{
		ImGui::OpenPopup("PhysicsPopup");
	}
	if (ImGui::BeginPopup("PhysicsPopup"))
	{
		DrawPhysicsPopup(girl);

		ImGui::EndPopup();
	}

	//ImGui::Separator();

	//const bool saveAll = ImGui::Button("Save All");
	//const bool resetAll = ImGui::Button("Reset All");

	//if (saveAll || resetAll)
	//{
	//	assert((girl.HasComponents<AnimationDeltas, MoveTargets>()));
	//	auto [deltas, targets] = girl.GetComponents<AnimationDeltas, MoveTargets>();

	//	if (saveAll)
	//	{
	//		LOG_IF_ERROR(GirlStatConfig::SerializeToJson(&deltas, &targets));
	//	}
	//	else
	//	{
	//		LOG_IF_ERROR(GirlStatConfig::SerializeToJson(&deltas, &targets));
	//	}
	//}
}

void GirlPhysicsEditor::DrawAnimationsPopup(Entity& girl)
{
	assert(girl.HasComponent<AnimationDeltas>());

	auto& deltas = girl.GetComponent<AnimationDeltas>();

	DrawFieldF(kIdleTimeLabel, deltas.idleTime, 0.01f);

	DrawFieldF(kLandingTimeLabel, deltas.landTime, 0.01f);

	DrawFieldF(kAttackATimeLabel, deltas.attackATime, 0.01f);

	DrawFieldF(kAttackBTimeLabel, deltas.attackBTime, 0.01f);

	DrawFieldF(kWalkDeltaXLabel, deltas.walkDeltaX, 0.01f);

	DrawFieldF(kJumpDeltaYLabel, deltas.jumpDeltaY, 0.01f);

	DrawFieldF(kFallDeltaYLabel, deltas.fallDeltaY, 0.01f);

	ImGui::Separator();

	if (ImGui::Button("Save"))
	{
		LOG_IF_ERROR(GirlStatConfig::SerializeToJson(&deltas, nullptr));
	}
	if (ImGui::Button("Reset"))
	{
		LOG_IF_ERROR(GirlStatConfig::DeserializeFromJson(&deltas, nullptr));
	}
	if (ImGui::Button("Defaults"))
	{
		deltas = kGirlBaseAnimationDeltas;
	}
}

void GirlPhysicsEditor::DrawPhysicsPopup(Entity& girl)
{
	assert(girl.HasComponent<MoveTargets>());

	auto& targets = girl.GetComponent<MoveTargets>();

	DrawFieldF(kAccelGroundLabel, targets.accelGround);

	DrawFieldF(kAccelAirLabel, targets.accelAir);

	DrawFieldF(kMaxWalkSpeedLabel, targets.maxSpeed);

	DrawFieldF(kJumpImpulseYLabel, targets.jumpVelY);

	//DrawFieldF(kBaseFrictionLabel, kGirlColliderFriction);

	//DrawFieldF(kLandFrictionLabel, kGirlColliderLandingFriction);

	//DrawFieldF(kWalkStopVelXLabel, kGirlWalkStopVelocityX);

	ImGui::Separator();

	if (ImGui::Button("Save"))
	{
		LOG_IF_ERROR(GirlStatConfig::SerializeToJson(nullptr, &targets));
	}
	if (ImGui::Button("Reset"))
	{
		LOG_IF_ERROR(GirlStatConfig::DeserializeFromJson(nullptr, &targets));
	}
	if (ImGui::Button("Defaults"))
	{
		targets = kGirlBaseMoveTargets;
	}
}

} // test

#endif