#pragma once
#include "Common.h"
#include <imgui.h>

class Entity;

namespace test {

class GirlPhysicsEditor
{
public:
	static constexpr const char* kIdleTimeLabel     = "Idle Time   ";
	static constexpr const char* kLandingTimeLabel  = "Landing Time";
	static constexpr const char* kAttackTimeLabel   = "Attack Time ";
	static constexpr const char* kWalkDeltaXLabel   = "Walk Delta X";
	static constexpr const char* kJumpDeltaYLabel   = "Jump Delta Y";
	static constexpr const char* kFallDeltaYLabel   = "Fall Delta Y";

	static constexpr const char* kAccelGroundLabel  = "Ground Acceleration ";
	static constexpr const char* kAccelAirLabel     = "Air Acceleration    ";
	static constexpr const char* kMaxWalkSpeedLabel = "Max Walk Speed X    ";
	static constexpr const char* kJumpImpulseYLabel = "Jump Impulse Y      ";
	static constexpr const char* kBaseFrictionLabel = "Base Friction       ";
	static constexpr const char* kLandFrictionLabel = "Landing Friction    ";
	static constexpr const char* kWalkStopVelXLabel = "Walk Stop Velocity X";

	static void Draw(Entity& girl);

private:
	static void DrawAnimationsPopup(Entity& girl);

	static void DrawPhysicsPopup(Entity& girl);

	static bool DrawFieldF(const char* label, float& var, float amount = 0.1f)
	{
		float f = var;

		if (ImGui::DragFloat(label, &f, amount))
		{
			var = f;
			return true;
		}

		return false;
	}
};

} // test


