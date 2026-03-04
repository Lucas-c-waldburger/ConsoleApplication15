#include "AttackAnimDataReference.h"


namespace test {

std::string_view AttackAnimDataReference::GetAttackAnimSeriesName() const
{
	assert(attackSeriesIdx_ < kAttackAnimSeriesNames.size());

	return kAttackAnimSeriesNames[attackSeriesIdx_];
}

float AttackAnimDataReference::GetAttackAnimChangeTime(const AnimationDeltas& deltas) const
{
	return (GetAttackAnimSeriesName() == kGirlAttackASeriesName)
		? deltas.attackATime
		: deltas.attackBTime;
}

void AttackAnimDataReference::Update(float dt)
{
	if (!inAttack_)
	{
		timeSinceLastAttackFinished_ += dt;

		if (timeSinceLastAttackFinished_ > kGirlAttackAltAnimWindowTime)
		{
			attackSeriesIdx_ = 0;
		}
	}
}

void AttackAnimDataReference::MarkNewAttack()
{
	if (!inAttack_ &&
		timeSinceLastAttackFinished_ <= kGirlAttackAltAnimWindowTime)
	{
		attackSeriesIdx_ = (attackSeriesIdx_ + 1) %
			kAttackAnimSeriesNames.size();

		timeSinceLastAttackFinished_ = 0.0f;
	}

	inAttack_ = true;
}

void AttackAnimDataReference::MarkAttackEnd()
{
	timeSinceLastAttackFinished_ = 0.0f;
	inAttack_ = false;
}






} // test