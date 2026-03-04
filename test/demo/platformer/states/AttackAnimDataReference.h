#pragma once
#include "../Common.h"
namespace test {

class AttackAnimDataReference
{
public:
	static constexpr std::array kAttackAnimSeriesNames = {
		kGirlAttackASeriesName,
		kGirlAttackBSeriesName
	};

	std::string_view GetAttackAnimSeriesName() const;
	float GetAttackAnimChangeTime(const AnimationDeltas& deltas) const;

	void Update(float dt);

	void MarkNewAttack();
	void MarkAttackEnd();

private:
	size_t attackSeriesIdx_ = 0;
	float timeSinceLastAttackFinished_ = 0.0f;
	bool inAttack_ = false;
};

} // test