#include "GirlPhysicsEditor.h"
#include "../../../ecs/Ecs.h"

namespace test {

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
}

void GirlPhysicsEditor::DrawAnimationsPopup(Entity& girl)
{
	assert(girl.HasComponent<AnimationDeltas>());

	auto& deltas = girl.GetComponent<AnimationDeltas>();

	DrawFieldF(kIdleTimeLabel, deltas.idleTime, 0.01f);

	DrawFieldF(kLandingTimeLabel, deltas.landTime, 0.01f);

	DrawFieldF(kAttackTimeLabel, deltas.attackTime, 0.01f);

	DrawFieldF(kWalkDeltaXLabel, deltas.walkDeltaX, 0.01f);

	DrawFieldF(kJumpDeltaYLabel, deltas.jumpDeltaY, 0.01f);

	DrawFieldF(kFallDeltaYLabel, deltas.fallDeltaY, 0.01f);

	if (ImGui::Button("Reset"))
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

	DrawFieldF(kBaseFrictionLabel, kGirlColliderFriction);

	DrawFieldF(kLandFrictionLabel, kGirlColliderLandingFriction);

	DrawFieldF(kWalkStopVelXLabel, kGirlWalkStopVelocityX);

	if (ImGui::Button("Reset"))
	{
		float tempVelX = targets.targetVelX;
		float tempDt = targets.dt;

		targets = kGirlBaseMoveTargets;

		targets.targetVelX = tempVelX;
		targets.dt = tempDt;
	}
}




} // test