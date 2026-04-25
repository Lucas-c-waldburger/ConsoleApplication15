#pragma once
#include "../GirlChecks.h"

namespace test {

struct CoyoteTime
{
	int frameAllowance = 8;
	int currentFrame = -1;

	void Update(const GirlState& state) noexcept
	{
		if (state.action.jumpIntent == InputState::Pressed)
		{
			currentFrame = 0;
		}

		if (currentFrame > -1)
		{
			if (currentFrame <= frameAllowance)
			{
				++currentFrame;
			}
			else
			{
				currentFrame = -1;
			}
		}
	}

	bool JumpBuffered() const noexcept
	{
		return currentFrame >= 0 && currentFrame <= frameAllowance;
	}
};

struct JumpAnimDelta
{
	float deltaY = 0.0f;

	void Update(const GirlState& state, const AnimationDeltas& deltas)
	{
		using enum GirlState::Animation;

		deltaY = deltas.jumpDeltaY;

		if (GirlInState(state, Jumping)
			.And(JumpIntentMatchesAny(state, InputState::Released, InputState::None)))
		{
			deltaY *= 0.5;
		}
	}
};

struct JumpQuery
{
	void Update(const GirlState& state, const AnimationDeltas& deltas) noexcept
	{
		coyoteTime.Update(state);
		jumpAnimDelta.Update(state, deltas);
	}

	bool JumpBuffered() const noexcept
	{
		return coyoteTime.JumpBuffered();
	}

	float GetJumpAnimDeltaY() const noexcept
	{
		return jumpAnimDelta.deltaY;
	}

	CoyoteTime coyoteTime;
	JumpAnimDelta jumpAnimDelta;
};



} // test