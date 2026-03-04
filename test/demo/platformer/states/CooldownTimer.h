#pragma once
#include "../../../../core/WeightGenerator.h";

namespace test {

struct CooldownTimer
{
	float duration = 0.0f;
	float elapsed = 0.0f;

	void Update(float dt) noexcept { if (Running() && !Ready()) { elapsed += dt; } }
	bool Ready() const noexcept { return elapsed >= duration; }
	bool Running() const noexcept { return elapsed >= 0.0f; }

	void Pause() noexcept { elapsed = -1.0f; }
	void Reset() noexcept { elapsed = 0.0f; }
};

struct FrameValueSpread
{
	std::vector<float> weightedValues;

	static FrameValueSpread Create(int frameCnt, float totalVal, float spread)
	{
		auto weights = WeightGenerator::GenerateGaussianWeights(frameCnt, 0.0f, spread);

		//FrameValueSpread{
		//	.weightedValues = 
		//}
	}
};


} // test