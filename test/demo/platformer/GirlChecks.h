#pragma once
#include "Common.h"
#include "../../../inputs/controller/GameController.h"

namespace test {

template <typename...Ts>
concept AllAnimStates = sizeof...(Ts) > 0 &&
	(std::same_as<std::remove_cvref_t<Ts>, GirlState::Animation> && ...);

struct GirlInStateImpl
{
	operator bool () const { return result; }

	template <typename Fn, typename...Args> 
		requires std::is_invocable_r_v<bool, Fn, const GirlState&, Args...>
	GirlInStateImpl& And(Fn&& fn, Args&&...args)
	{
		result = result && std::invoke(fn, girlState, std::forward<Args>(args)...);
		return *this;
	}
	template <typename Fn, typename...Args>
		requires std::is_invocable_r_v<bool, Fn, const GirlState&, Args...>
	GirlInStateImpl& Or(Fn&& fn, Args&&...args)
	{
		result = result || std::invoke(fn, girlState, std::forward<Args>(args)...);
		return *this;
	}

	//GirlInStateImpl& And(GirlInState)

	GirlInStateImpl& And(bool b) { result = result && b; return *this; }
	GirlInStateImpl& Or(bool b)  { result = result || b; return *this; }

protected:
	template <AllAnimStates...Ts>
	GirlInStateImpl(const GirlState& gs, bool negate, Ts...tests) : girlState(gs)
	{
		result = ((girlState.animation == tests) || ...);
		if (negate)
		{
			result = !result;
		}
	}

private:
	const GirlState& girlState;
	bool result = false;
};

struct GirlInState : public GirlInStateImpl 
{ 
	template <AllAnimStates...Ts>
	GirlInState(const GirlState& gs, Ts...tests) : 
		GirlInStateImpl(gs, false, tests...) {}
};
struct GirlNotInState : public GirlInStateImpl
{
	template <AllAnimStates...Ts>
	GirlNotInState(const GirlState& gs, Ts...tests) :
		GirlInStateImpl(gs, true, tests...) {}
};

template <AllAnimStates...Ts> 
inline bool IsGirlInState(const GirlState& state, Ts...states)
{
	return ((state.animation == states) || ...);
}

inline bool IsGirlAtEndOfAnimationSeries(const SpriteAnimationComponent& anim, 
										 size_t endSubMod = 0)
{
	const size_t idxEnd = (endSubMod > anim.index.max) ? 0 : anim.index.max - endSubMod;

	return anim.index.current == idxEnd;
}

inline bool IsGirlOnGround(const GirlState& state)
{
	return state.collidingCategories[ObjectCategory::Ground] > 0;
}
inline bool IsThumbstickEngaged(const GameControllerState& gc)
{
	if (gc.joystickID == GameController::kInvalidJoystickID)
	{
		return false;
	}

	const auto& gcAxis =
		gc.inputs[GameControllerInputSource::LeftStickAxis];

	const int axisX = std::abs(static_cast<int>(gcAxis.value.axis.x));
	const int axisY = std::abs(static_cast<int>(gcAxis.value.axis.y));

	return (gcAxis.state == InputState::Pressed ||
			gcAxis.state == InputState::Held) &&
			axisX > kGirlControllerAxisDeadzone ||
			axisY > kGirlControllerAxisDeadzone;
}
inline bool IsGirlJumpFlagged(const GirlState& state)
{
	return state.action.jumpIntent == InputState::Pressed;
}
inline bool IsGirlAttackFlagged(const GirlState& state)
{
	return state.action.attackIntent == InputState::Pressed;
}
inline bool IsGirlDashFlagged(const GirlState& state)
{
	return state.action.dashIntent == InputState::Pressed;
}
inline bool IsGirlCurrentlyJumping(const GirlState& state)
{
	return state.animation == GirlState::Animation::Jumping;
}
inline bool IsGirlCurrentlyFalling(const GirlState& state)
{
	return state.animation == GirlState::Animation::Falling;
}
inline bool IsGirlCurrentlyIdle(const GirlState& state)
{
	return state.animation == GirlState::Animation::Idle;
}
inline bool IsGirlCurrentlyLanding(const GirlState& state)
{
	return state.animation == GirlState::Animation::Landing;
}
inline bool IsGirlCurrentlyWalking(const GirlState& state)
{
	return state.animation == GirlState::Animation::Walking;
}
inline bool IsGirlCurrentlyAttacking(const GirlState& state)
{
	return state.animation == GirlState::Animation::Attacking;
}
inline bool IsGirlCurrentlyRolling(const GirlState& state)
{
	return state.animation == GirlState::Animation::Rolling;
}
inline bool IsGirlCurrentlyDashing(const GirlState& state)
{
	return state.animation == GirlState::Animation::Dashing;
}
inline bool WalkVelocityXUnderStopThreshold(RigidBody& rigid)
{
	return std::abs(rigid.body.GetData().GetLinearVelocity().x) <
		   kGirlWalkStopVelocityX;
}
inline bool ShouldAttack(const GirlState& state, const SpriteAnimationComponent& anim)
{
	using enum GirlState::Animation;

	if (IsGirlInState(state, Attacking, Landing))
	{
		return IsGirlAtEndOfAnimationSeries(anim, 1);
	}

	if (IsGirlInState(state, Dashing))
	{
		return IsGirlAtEndOfAnimationSeries(anim);
	}

	return true;
}

inline bool ShouldJump(GirlState& state, const SpriteAnimationComponent& anim)
{
	using enum GirlState::Animation;

	return IsGirlOnGround(state) &&
		  GirlNotInState(state, Jumping) &&
		  GirlNotInState(state, Attacking).Or(IsGirlAtEndOfAnimationSeries(anim, 1)) &&
		  GirlNotInState(state, Dashing).Or(IsGirlAtEndOfAnimationSeries(anim));
}

inline bool JumpButtonReleased(GirlState& state)
{
	return state.action.jumpIntent == InputState::Released;
}

inline bool SpriteChanged(const SpriteAnimationComponent& animCopy,
						  const Entity& girl)
{
	return animCopy != girl.GetComponent<SpriteAnimationComponent>();
}





} // test