//#pragma once
//#include "../Common.h"
//
//#define CHECK_GIRL_VALID(ctx) do { \
//	if (!ctx.girl.IsValid()) { return false; } \
//} while(0)
//
//namespace test {
//
//inline bool IsGirlJumping(const GirlState& state)
//{
//	return state.animation == GirlState::Animation::Jumping;
//}
//inline bool IsGirlFalling(const GirlState& state)
//{
//	return state.animation == GirlState::Animation::Falling;
//}
//inline bool IsGirlIdle(const GirlState& state)
//{
//	return state.animation == GirlState::Animation::Idle;
//}
//inline bool IsGirlLanding(const GirlState& state)
//{
//	return state.animation == GirlState::Animation::Landing;
//}
//inline bool IsGirlWalking(const GirlState& state)
//{
//	return state.animation == GirlState::Animation::Walking;
//}
//inline bool IsGirlAttacking(const GirlState& state)
//{
//	return state.animation == GirlState::Animation::Attacking;
//}
//inline bool IsGirlRolling(const GirlState& state)
//{
//	return state.animation == GirlState::Animation::Rolling;
//}
//
//inline bool IsGirlOnGround(const GirlState& girlState)
//{
//	return girlState.collidingCategories[ObjectCategory::Ground] > 0;
//}
//
//inline bool IsGirlOnGround(const GirlStateContext& ctx)
//{
//	CHECK_GIRL_VALID(ctx);
//
//	const auto& girlState = ctx.girl.GetComponent<GirlState>();
//
//	return girlState.collidingCategories[ObjectCategory::Ground] > 0;
//}
//
//inline bool IsGirlAtEndOfAnimationSeries(const GirlStateContext& ctx)
//{
//	CHECK_GIRL_VALID(ctx);
//
//	const auto& anim = ctx.girl.GetComponent<SpriteAnimationComponent>();
//
//	return anim.index.current == anim.index.max;
//}
//
//inline bool TimeInAnimCrossesThreshold(const GirlStateContext& ctx, 
//									   float threshold, float mod = 0.0f)
//{
//	return ctx.timeInCurrentAnimFrame >= threshold + mod;
//}
//
//inline bool LandingAnimationCompleted(const GirlStateContext& ctx)
//{
//	CHECK_GIRL_VALID(ctx);
//
//	if (!IsGirlAtEndOfAnimationSeries(ctx))
//	{
//		return false;
//	}
//
//	const auto& animDeltas = ctx.girl.GetComponent<AnimationDeltas>();
//
//	return TimeInAnimCrossesThreshold(ctx, animDeltas.landTime, animDeltas.landTimeEndMod);
//}
//
//inline bool AttackAnimationCompleted(const GirlStateContext& ctx)
//{
//	CHECK_GIRL_VALID(ctx);
//
//	if (!IsGirlAtEndOfAnimationSeries(ctx))
//	{
//		return false;
//	}
//
//	const auto& animDeltas = ctx.girl.GetComponent<AnimationDeltas>();
//
//	return TimeInAnimCrossesThreshold(ctx, animDeltas.landTime, animDeltas.landTimeEndMod);
//}
//
//inline bool GirlMoved(const GirlStateContext& ctx)
//{
//	CHECK_GIRL_VALID(ctx);
//
//	const auto& tf = ctx.girl.GetComponent<Transform>();
//
//	return !(EqualsWithTolerance(tf.position.x, ctx.lastPosition.x) &&
//			 EqualsWithTolerance(tf.position.y, ctx.lastPosition.y));
//}
//
//inline bool IsGirlJumpFlagged(const GirlState& girlState)
//{
//	return girlState.jumpInitiated;
//}
//
//inline bool IsGirlAttackFlagged(const GirlState& girlState)
//{
//	return girlState.attackInitiated;
//}
//
//inline bool ShouldJump(const GirlStateContext& ctx)
//{
//	CHECK_GIRL_VALID(ctx);
//
//	const auto [girlState, anim] = 
//		ctx.girl.GetComponents<GirlState, SpriteAnimationComponent>();
//
//	return IsGirlJumpFlagged(girlState) &&
//		   IsGirlOnGround(girlState) &&
//		   !IsGirlJumping(girlState) &&
//		  (!IsGirlAttacking(girlState) ||
//		   anim.index.current >= anim.index.max - 1);
//}
//
//inline bool ShouldAttack(const GirlStateContext& ctx)
//{
//	CHECK_GIRL_VALID(ctx);
//
//	const auto [girlState, anim] =
//		ctx.girl.GetComponents<GirlState, SpriteAnimationComponent>();
//
//	if (!girlState.attackInitiated)
//	{
//		return false;
//	}
//
//	if (IsGirlAttacking(girlState) || IsGirlJumping(girlState))
//	{
//		return anim.index.current >= anim.index.max - 1;
//	}
//
//	return true;
//}
//
//inline bool IsYPosIncreasing(const GirlStateContext& ctx)
//{
//	CHECK_GIRL_VALID(ctx);
//
//	const auto& tf = ctx.girl.GetComponent<Transform>();
//
//	return tf.position.y > ctx.lastPosition.y;
//}
//
//inline bool IsThumbstickEngaged(const GirlStateContext& ctx)
//{
//	CHECK_GIRL_VALID(ctx);
//
//	const auto& gc = ctx.girl.GetComponent<GameControllerState>();
//
//	if (gc.joystickID == -1)
//	{
//		return false;
//	}
//
//	const auto& gcAxis =
//		gc.inputs[GameControllerInputSource::LeftStickAxis];
//
//	const int axisX = std::abs(static_cast<int>(gcAxis.value.axis.x));
//	const int axisY = std::abs(static_cast<int>(gcAxis.value.axis.y));
//
//	return (gcAxis.state == InputState::Pressed ||
//			gcAxis.state == InputState::Held) &&
//		   (axisX > kGirlControllerAxisDeadzone ||
//			axisY > kGirlControllerAxisDeadzone);
//}
//
//inline bool WalkVelocityXUnderStopThreshold(const GirlStateContext& ctx)
//{
//	CHECK_GIRL_VALID(ctx);
//
//	const auto& rigid = ctx.girl.GetComponent<RigidBody>();
//
//	return std::abs(rigid.body.GetData().GetLinearVelocity().x) <
//		   kGirlWalkStopVelocityX;
//}
//
//inline void StopWalkVelocityX(GirlStateContext& ctx)
//{
//	assert(ctx.girl.IsValid());
//
//	auto& rigid = ctx.girl.GetComponent<RigidBody>();
//	auto& body = WriteAccessor<B2Body>{}(rigid.body);
//
//	body.SetLinearVelocity({ 0.0f, body.GetLinearVelocity().y });
//}
//
//} // test