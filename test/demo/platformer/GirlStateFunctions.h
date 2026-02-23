#pragma once
#include "Common.h"
#include "EntityStateMachine.h"

namespace test {

using GirlStateNode = EntityStateNode<GirlState::Animation, GirlStateContext>;

void OnGirlJumpStateEnter(GirlStateContext& ctx, Entity& girl);
GirlStateNode::EvaluationResponse OnGirlJumpStateEval(GirlStateContext& ctx, Entity& girl);
void OnGirlJumpStateIntent(GirlStateContext& ctx, Entity& girl);
void OnGirlJumpStateAnimation(GirlStateContext& ctx, Entity& girl);

void OnGirlFallStateEnter(GirlStateContext& ctx, Entity& girl);
GirlStateNode::EvaluationResponse OnGirlFallStateEval(GirlStateContext& ctx, Entity& girl);
void OnGirlFallStateIntent(GirlStateContext& ctx, Entity& girl);
void OnGirlFallStateAnimation(GirlStateContext& ctx, Entity& girl);
void OnGirlFallStateExit(GirlStateContext&, Entity& girl);

void OnGirlIdleStateEnter(GirlStateContext&, Entity& girl);
GirlStateNode::EvaluationResponse OnGirlIdleStateEval(GirlStateContext& ctx, Entity& girl);
void OnGirlIdleStateAnimation(GirlStateContext& ctx, Entity& girl);

void OnGirlWalkStateEnter(GirlStateContext&, Entity& girl);
GirlStateNode::EvaluationResponse OnGirlWalkStateEval(GirlStateContext& ctx, Entity& girl);
void OnGirlWalkStateIntent(GirlStateContext& ctx, Entity& girl);
void OnGirlWalkStateAnimation(GirlStateContext& ctx, Entity& girl);

void OnGirlLandingStateEnter(GirlStateContext& ctx, Entity& girl);
GirlStateNode::EvaluationResponse OnGirlLandingStateEval(GirlStateContext& ctx, Entity& girl);
void OnGirlLandingStateIntent(GirlStateContext& ctx, Entity& girl);
void OnGirlLandingStateAnimation(GirlStateContext& ctx, Entity& girl);

static constexpr GirlStateNode kGirlJumpStateNode{
	.onEnter = &OnGirlJumpStateEnter,
	.evaluateState = &OnGirlJumpStateEval,
	.updateIntents = &OnGirlJumpStateIntent,
	.updateAnimations = &OnGirlJumpStateAnimation
};
static constexpr GirlStateNode kGirlFallStateNode{
	.onEnter = &OnGirlFallStateEnter,
	.evaluateState = &OnGirlFallStateEval,
	.updateIntents = &OnGirlFallStateIntent,
	.updateAnimations = &OnGirlFallStateAnimation,
	.onExit = &OnGirlFallStateExit
};
static constexpr GirlStateNode kGirlIdleStateNode{
	.onEnter = &OnGirlIdleStateEnter,
	.evaluateState = &OnGirlIdleStateEval,
	.updateAnimations = &OnGirlIdleStateAnimation
};
static constexpr GirlStateNode kGirlLandStateNode{
	.onEnter = &OnGirlLandingStateEnter,
	.evaluateState = &OnGirlLandingStateEval,
	.updateIntents = &OnGirlLandingStateIntent,
	.updateAnimations = &OnGirlLandingStateAnimation
};
static constexpr GirlStateNode kGirlWalkStateNode{
	.onEnter = &OnGirlWalkStateEnter,
	.evaluateState = &OnGirlWalkStateEval,
	.updateIntents = &OnGirlWalkStateIntent,
	.updateAnimations = &OnGirlWalkStateAnimation
};

} // test