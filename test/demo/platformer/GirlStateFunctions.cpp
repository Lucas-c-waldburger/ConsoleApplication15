#include "GirlStateFunctions.h"
#include "Common.h"
#include "../../../inputs/controller/GameController.h"
#include "../../../ecs/Ecs.h"

namespace test {

namespace {

constexpr bool HasMoveIntent(const GirlIntent& intent)
{
	return !(EqualsWithTolerance(intent.moveIntent.x, 0.0f) &&
			 EqualsWithTolerance(intent.moveIntent.y, 0.0f));
}

}

void HandleGirlAxisMovement(GirlStateContext& ctx, Entity& girl)
{
	auto [rigid, state, intent] = girl.GetComponents<RigidBody, GirlState, GirlIntent>();

	const float normedX = intent.moveIntent.x / static_cast<float>(GameController::kAxisMax);

	ctx.targets.targetVelX = normedX * ctx.targets.maxSpeed;

	float moveImpulseX = ComputeMoveImpulseX(rigid, state, ctx.targets);

	rigid.forceRequests.impulses.emplace_back(
		Force{ .value = { moveImpulseX, 0.0f } });
}

void OnGirlJumpStateEnter(GirlStateContext& ctx, Entity& girl)
{
	auto [anim, state, rigid] =
		girl.GetComponents<SpriteAnimationComponent, GirlState, RigidBody>();

	anim.spriteSeriesName = "girl_jump";
	anim.index.current = 0;
	state.animation = GirlState::Animation::Jumping;

	ctx.flags |= GirlStateContext::Flag::PerformJumpPressedIntent;
}

void OnGirlJumpStateIntent(GirlStateContext& ctx, Entity& girl)
{
	if ((ctx.flags & GirlStateContext::Flag::PerformJumpPressedIntent) != 0)
	{
		auto& rigid = girl.GetComponent<RigidBody>();

		const float jumpImpulseY = ComputeJumpImpulseY(rigid, ctx.targets);

		rigid.forceRequests.impulses.emplace_back(
			Force{ .value = { 0.0f, -jumpImpulseY } });

		ctx.flags &= ~(GirlStateContext::Flag::PerformJumpPressedIntent);
	}

	if (HasMoveIntent(girl.GetComponent<GirlIntent>()))
	{
		HandleGirlAxisMovement(ctx, girl);
	}
}

GirlStateNode::EvaluationResponse
OnGirlJumpStateEval(GirlStateContext& ctx, Entity& girl)
{
	const auto [tf, state] = girl.GetComponents<Transform, GirlState>();

	if (tf.position.y > ctx.deltas.lastPosition.y)
	{
		return GirlState::Animation::Falling;
	}
	if (state.collidingCategories[ObjectCategory::Ground] > 0)
	{
		return GirlState::Animation::Landing;
	}

	return GirlState::Animation::Jumping;
}

void OnGirlJumpStateAnimation(GirlStateContext& ctx, Entity& girl)
{
	const auto& constGirl = girl;
	const auto& animIdx = girl.GetComponent<SpriteAnimationComponent>().index;

	if (ctx.deltas.deltaAnimMove.y >= kGirlJumpAnimChangeYDelta &&
		animIdx.current < animIdx.max)
	{
		auto& anim = girl.GetComponent<SpriteAnimationComponent>();
		assert(anim.spriteSeriesName == "girl_jump");

		++anim.index;
	}
}

void OnGirlFallStateEnter(GirlStateContext&, Entity& girl)
{
	auto [anim, state, rigid] =
		girl.GetComponents<SpriteAnimationComponent, GirlState, RigidBody>();

	anim.spriteSeriesName = "girl_fall";
	anim.index.current = 0;
	state.animation = GirlState::Animation::Falling;

	WriteAccessor<B2Body>{}(rigid.body).SetGravityScale(2.5f);
}

void OnGirlFallStateAnimation(GirlStateContext& ctx, Entity& girl)
{
	const auto& constGirl = girl;
	const auto& anim = girl.GetComponent<SpriteAnimationComponent>();

	if (ctx.deltas.deltaAnimMove.y >= kGirlFallAnimChangeYDelta &&
		anim.index.current < anim.index.max)
	{
		auto& anim = girl.GetComponent<SpriteAnimationComponent>();
		assert(anim.spriteSeriesName == "girl_fall");

		++anim.index;
	}
}

GirlStateNode::EvaluationResponse
OnGirlFallStateEval(GirlStateContext&, Entity& girl)
{
	const auto& state = girl.GetComponent<GirlState>();

	if (state.collidingCategories[ObjectCategory::Ground] > 0)
	{
		return GirlState::Animation::Landing;
	}

	return GirlState::Animation::Falling;
}

void OnGirlFallStateIntent(GirlStateContext& ctx, Entity& girl)
{
	if (HasMoveIntent(girl.GetComponent<GirlIntent>()))
	{
		HandleGirlAxisMovement(ctx, girl);
	}
}

void OnGirlFallStateExit(GirlStateContext&, Entity& girl)
{
	auto& rigid = girl.GetComponent<RigidBody>();
	auto& body = WriteAccessor<B2Body>{}(rigid.body);

	body.SetGravityScale(1.0f);
	body.SetLinearVelocity({ 0.0f, 0.0f });
}

void OnGirlIdleStateEnter(GirlStateContext&, Entity& girl)
{
	auto [anim, state] =
		girl.GetComponents<SpriteAnimationComponent, GirlState>();

	anim.spriteSeriesName = "girl_idle";
	anim.index.current = 0;
	state.animation = GirlState::Animation::Idle;
}

void OnGirlIdleStateAnimation(GirlStateContext& ctx, Entity& girl)
{
	if (ctx.deltas.deltaAnimTime >= kGirlIdleAnimChangeTime)
	{
		auto& anim = girl.GetComponent<SpriteAnimationComponent>();
		assert(anim.spriteSeriesName == "girl_idle");
		
		++anim.index;
	}
}

GirlStateNode::EvaluationResponse
OnGirlIdleStateEval(GirlStateContext& ctx, Entity& girl)
{
	const auto [tf, state] = girl.GetComponents<Transform, GirlState>();

	auto eval = GirlStateContext::Evaluate(girl, ctx);

	if (!eval.onGround)
	{
		return GirlState::Animation::Falling;
	}
	if (eval.jumpPressed || 
		(ctx.flags & GirlStateContext::PerformJumpPressedIntent))
	{
		return GirlState::Animation::Jumping;
	}
	if (eval.attackPressed)
	{
		return GirlState::Animation::Attacking;
	}
	if (eval.moved || eval.thumbstickEngaged)
	{
		return GirlState::Animation::Walking;
	}

	return GirlState::Animation::Idle;
}

void OnGirlWalkStateEnter(GirlStateContext&, Entity& girl)
{
	auto [anim, state] = girl.GetComponents<SpriteAnimationComponent, GirlState>();

	anim.spriteSeriesName = "girl_walk";
	anim.index.current = 0;
	state.animation = GirlState::Animation::Walking;
}

void OnGirlWalkStateIntent(GirlStateContext& ctx, Entity& girl)
{
	if (HasMoveIntent(girl.GetComponent<GirlIntent>()))
	{
		HandleGirlAxisMovement(ctx, girl);
	}
}

void OnGirlWalkStateAnimation(GirlStateContext& ctx, Entity& girl)
{
	if (ctx.deltas.deltaAnimMove.x >= kGirlWalkAnimChangeXDelta)
	{
		auto& anim = girl.GetComponent<SpriteAnimationComponent>();
		assert(anim.spriteSeriesName == "girl_walk");

		++anim.index;
	}
}

GirlStateNode::EvaluationResponse
OnGirlWalkStateEval(GirlStateContext& ctx, Entity& girl)
{
	auto eval = GirlStateContext::Evaluate(girl, ctx);

	if (!eval.onGround)
	{
		return GirlState::Animation::Falling;
	}
	if (eval.jumpPressed)
	{
		return GirlState::Animation::Jumping;
	}
	if (eval.attackPressed)
	{
		return GirlState::Animation::Attacking;
	}
	if (!(eval.moved || eval.thumbstickEngaged))
	{
		return GirlState::Animation::Idle;
	}

	return GirlState::Animation::Walking;
}

void OnGirlLandingStateEnter(GirlStateContext& ctx, Entity& girl)
{
	auto [anim, state] = girl.GetComponents<SpriteAnimationComponent, GirlState>();

	anim.spriteSeriesName = "girl_land";
	anim.index.current = 0;
	state.animation = GirlState::Animation::Landing;

	ctx.flags |= GirlStateContext::Flag::PerformLandingIntent;
}

GirlStateNode::EvaluationResponse
OnGirlLandingStateEval(GirlStateContext& ctx, Entity& girl)
{
	const auto& cGirl = girl;
	const auto [state, anim] = cGirl.GetComponents<GirlState, SpriteAnimationComponent>();

	if (state.collidingCategories[ObjectCategory::Ground] == 0)
	{
		return GirlState::Animation::Falling;
	}
	if (anim.index.current == anim.index.max)
	{
		return { GirlState::Animation::Idle, false };
	}

	return GirlState::Animation::Landing;
}

void OnGirlLandingStateIntent(GirlStateContext& ctx, Entity& girl)
{
	if ((ctx.flags & GirlStateContext::Flag::PerformLandingIntent) == 0)
	{
		return;
	}

	auto& rigid = girl.GetComponent<RigidBody>();

	WriteAccessor<B2Body>{}(rigid.body).SetLinearVelocity({ 0.0f, 0.0f });

	ctx.flags &= ~(GirlStateContext::Flag::PerformLandingIntent);
}

void OnGirlLandingStateAnimation(GirlStateContext& ctx, Entity& girl)
{
	if (ctx.deltas.deltaAnimTime < kGirlLandAnimChangeTime)
	{
		return;
	}

	const auto& cGirl = girl;
	const auto animCopy = cGirl.GetComponent<SpriteAnimationComponent>();

	if (animCopy.index.current < animCopy.index.max)
	{
		auto& anim = girl.GetComponent<SpriteAnimationComponent>();
		assert(anim.spriteSeriesName == "girl_land");

		++anim.index;
	}
}


} // test