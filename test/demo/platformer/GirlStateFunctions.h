//#pragma once
//#include "Common.h"
//#include "EntityStateMachine.h"
//
//namespace test {
//
//using GirlStateNode = EntityStateNode<GirlState::Animation, GirlStateContextOld>;
//
//void OnGirlJumpStateEnter(GirlStateContextOld& ctx, Entity& girl);
//GirlStateNode::EvaluationResponse OnGirlJumpStateEval(GirlStateContextOld& ctx, Entity& girl);
//void OnGirlJumpStateIntent(GirlStateContextOld& ctx, Entity& girl);
//void OnGirlJumpStateAnimation(GirlStateContextOld& ctx, Entity& girl);
//
//void OnGirlFallStateEnter(GirlStateContextOld& ctx, Entity& girl);
//GirlStateNode::EvaluationResponse OnGirlFallStateEval(GirlStateContextOld& ctx, Entity& girl);
//void OnGirlFallStateIntent(GirlStateContextOld& ctx, Entity& girl);
//void OnGirlFallStateAnimation(GirlStateContextOld& ctx, Entity& girl);
//void OnGirlFallStateExit(GirlStateContextOld&, Entity& girl);
//
//void OnGirlIdleStateEnter(GirlStateContextOld&, Entity& girl);
//GirlStateNode::EvaluationResponse OnGirlIdleStateEval(GirlStateContextOld& ctx, Entity& girl);
//void OnGirlIdleStateAnimation(GirlStateContextOld& ctx, Entity& girl);
//
//void OnGirlWalkStateEnter(GirlStateContextOld&, Entity& girl);
//GirlStateNode::EvaluationResponse OnGirlWalkStateEval(GirlStateContextOld& ctx, Entity& girl);
//void OnGirlWalkStateIntent(GirlStateContextOld& ctx, Entity& girl);
//void OnGirlWalkStateAnimation(GirlStateContextOld& ctx, Entity& girl);
//
//void OnGirlLandingStateEnter(GirlStateContextOld& ctx, Entity& girl);
//GirlStateNode::EvaluationResponse OnGirlLandingStateEval(GirlStateContextOld& ctx, Entity& girl);
//void OnGirlLandingStateIntent(GirlStateContextOld& ctx, Entity& girl);
//void OnGirlLandingStateAnimation(GirlStateContextOld& ctx, Entity& girl);
//
//static constexpr GirlStateNode kGirlJumpStateNode{
//	.onEnter = &OnGirlJumpStateEnter,
//	.evaluateState = &OnGirlJumpStateEval,
//	.updateIntents = &OnGirlJumpStateIntent,
//	.updateAnimations = &OnGirlJumpStateAnimation
//};
//static constexpr GirlStateNode kGirlFallStateNode{
//	.onEnter = &OnGirlFallStateEnter,
//	.evaluateState = &OnGirlFallStateEval,
//	.updateIntents = &OnGirlFallStateIntent,
//	.updateAnimations = &OnGirlFallStateAnimation,
//	.onExit = &OnGirlFallStateExit
//};
//static constexpr GirlStateNode kGirlIdleStateNode{
//	.onEnter = &OnGirlIdleStateEnter,
//	.evaluateState = &OnGirlIdleStateEval,
//	.updateAnimations = &OnGirlIdleStateAnimation
//}; 
//static constexpr GirlStateNode kGirlLandStateNode{
//	.onEnter = &OnGirlLandingStateEnter,
//	.evaluateState = &OnGirlLandingStateEval,
//	.updateIntents = &OnGirlLandingStateIntent,
//	.updateAnimations = &OnGirlLandingStateAnimation
//};
//static constexpr GirlStateNode kGirlWalkStateNode{
//	.onEnter = &OnGirlWalkStateEnter,
//	.evaluateState = &OnGirlWalkStateEval,
//	.updateIntents = &OnGirlWalkStateIntent,
//	.updateAnimations = &OnGirlWalkStateAnimation
//};
//
//} // test