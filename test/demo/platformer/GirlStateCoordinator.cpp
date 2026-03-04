//#include "GirlStateCoordinator.h"
//#include "../../../events/EventBus2.h"
//#include "../../../inputs/controller/GameController.h"
//#include "states/GirlUtils.h"
//
//namespace test {
//
//GirlStateCoordinator::GirlStateCoordinator(Entity_t girlId, EventBus2& bus) :
//	girlId_(girlId), stateHandlers_(bus)
//{
//	stateHandlers_.RegisterState<GirlIdleStateHandler>();
//	stateHandlers_.RegisterState<GirlWalkStateHandler>();
//	stateHandlers_.RegisterState<GirlJumpStateHandler>();
//	stateHandlers_.RegisterState<GirlFallStateHandler>();
//	stateHandlers_.RegisterState<GirlLandStateHandler>();
//	stateHandlers_.RegisterState<GirlAttackStateHandler>();
//}
//
//void GirlStateCoordinator::Update(float dt)
//{
//	auto girl = ECS::GetEntityByID(girlId_);
//	if (!girl.IsValid())
//	{
//		return;
//	}
//
//	stateHandlers_.Upkeep(girl, dt);
//
//	auto& ctx = stateHandlers_.GetContext();
//	const auto activeState = stateHandlers_.GetActiveState();
//
//	if (ShouldJump(ctx))
//	{
//		stateHandlers_.SetState(State::Jumping);
//	}
//	else if (ShouldAttack(ctx))
//	{
//		stateHandlers_.SetState(State::Attacking);
//	}
//	else
//	{
//		switch (activeState)
//		{
//		case State::Jumping:
//		{
//			if (IsYPosIncreasing(ctx))
//			{
//				if (IsGirlOnGround(ctx))
//				{
//					stateHandlers_.SetState(State::Landing);
//				}
//				else
//				{
//					stateHandlers_.SetState(State::Falling);
//				}
//			}
//			break;
//		}
//		case State::Falling:
//		{
//			if (IsGirlOnGround(ctx))
//			{
//				stateHandlers_.SetState(State::Landing);
//			}
//			break;
//		}
//		case State::Landing:
//		{
//			if (!IsGirlOnGround(ctx))
//			{
//				stateHandlers_.SetState(State::Falling);
//			}
//			else if (LandingAnimationCompleted(ctx))
//			{
//				stateHandlers_.SetState(State::Idle);
//			}
//			break;
//		}
//		case State::Idle:
//		{
//			if (!IsGirlOnGround(ctx))
//			{
//				stateHandlers_.SetState(State::Falling);
//			}
//			else if (GirlMoved(ctx))
//			{
//				stateHandlers_.SetState(State::Walking);
//			}
//			break;
//		}
//		case State::Walking:
//		{
//			if (!IsGirlOnGround(ctx))
//			{
//				stateHandlers_.SetState(State::Falling);
//			}
//			else if (WalkVelocityXUnderStopThreshold(ctx) &&
//					!IsThumbstickEngaged(ctx))
//			{
//				StopWalkVelocityX(ctx);
//			}
//
//			if (!GirlMoved(ctx))
//			{
//				stateHandlers_.SetState(State::Idle);
//			}
//			break;
//		}
//		case State::Attacking:
//		{
//			if (IsGirlAtEndOfAnimationSeries(ctx))
//			{
//				const auto& attackHandler = 
//					stateHandlers_.GetStateHandler<GirlAttackStateHandler>();
//
//				if (TimeInAnimCrossesThreshold(ctx,
//					attackHandler.GetAttackAnimChangeTime(ctx)))
//				{
//					if (IsGirlOnGround(ctx))
//					{
//						stateHandlers_.SetState(State::Idle);
//					}
//					else
//					{
//						stateHandlers_.SetState(State::Falling);
//					}
//				}
//			}
//			break;
//		}
//		}
//
//		HandleAxisMoveIntent();
//
//		stateHandlers_.UpdateAnimation();
//		UpdateSpriteFacingSide();
//
//		stateHandlers_.Cleanup();
//	}
//}
//
//void GirlStateCoordinator::HandleAxisMoveIntent()
//{
//	auto& ctx = stateHandlers_.GetContext();
//	if (!ctx.girl.IsValid())
//	{
//		return;
//	}
//
//	auto [girlState, rigid, targets] =
//		ctx.girl.GetComponents<GirlState, RigidBody, MoveTargets>();
//
//	if (!girlState.axisMoveIntentX.has_value())
//	{
//		return;
//	}
//
//	const float intentX = static_cast<float>(*girlState.axisMoveIntentX);
//	girlState.axisMoveIntentX.reset();
//
//	if (stateHandlers_.GetActiveState() == State::Attacking)
//	{
//		return;
//	}
//
//	const float normedX = intentX / static_cast<float>(GameController::kAxisMax);
//
//	targets.targetVelX = normedX * targets.maxSpeed;
//
//	float moveImpulseX = ComputeMoveImpulseX(rigid, girlState, targets, ctx.dt);
//	if (stateHandlers_.GetActiveState() == State::Landing)
//	{
//		moveImpulseX /= 2.0f;
//	}
//
//	rigid.forceRequests.impulses.emplace_back(
//		Force{ .value = { moveImpulseX, 0.0f } });
//}
//
//void GirlStateCoordinator::UpdateSpriteFacingSide()
//{
//	auto& ctx = stateHandlers_.GetContext();
//	if (!ctx.girl.IsValid())
//	{
//		return;
//	}
//	if (stateHandlers_.GetActiveState() == State::Landing)
//	{
//		return;
//	}
//
//	auto [tf, rend] = ctx.girl.GetComponents<Transform, SpriteRenderableComponent>();
//
//	float xMoveDelta = ctx.lastPosition.x - tf.position.x;
//
//	rend.profile.flip = (tf.position.x < ctx.lastPosition.x) ? SDL_FLIP_HORIZONTAL :
//						(tf.position.x > ctx.lastPosition.x) ? SDL_FLIP_NONE :
//															   rend.profile.flip;
//}
//
//
//} // test
//
//
//
//
//
//
//
//
//
//
//
//
////#include "../../../systems/SystemManager.h"
////
////namespace test {
////
////// GirlStateCoordinator systems
////void GirlStateCoordinator::EvaluationSystem::UpdateImpl(Entity& girl, float dt)
////{
////	parentCoordinator_->UpdateStateContext(girl, dt);
////	parentCoordinator_->stateMachine_.Evaluate(girl);
////
////	girl.GetComponent<GirlState>().animation = parentCoordinator_->stateMachine_.current;
////}
////
////void GirlStateCoordinator::IntentSystem::UpdateImpl(Entity& girl, float)
////{
////	parentCoordinator_->stateMachine_.UpdateIntents(girl);
////}
////
////void GirlStateCoordinator::AnimationSystem::UpdateImpl(Entity& girl, float dt)
////{
////	parentCoordinator_->stateMachine_.UpdateAnimations(girl);
////	UpdateSpriteFacingSide(girl, parentCoordinator_->stateMachine_.context.deltas.lastPosition);
////	parentCoordinator_->ResolveStateContext(girl);
////}
////
////void GirlStateCoordinator::AnimationSystem::UpdateSpriteFacingSide(Entity& girl, SDL_FPoint lastPos)
////{
////	auto [tf, rend] = girl.GetComponents<Transform, SpriteRenderableComponent>();
////
////	float xMoveDelta = lastPos.x - tf.position.x;
////
////	rend.profile.flip = (tf.position.x < lastPos.x) ? SDL_FLIP_HORIZONTAL :
////						(tf.position.x > lastPos.x) ? SDL_FLIP_NONE :
////													  rend.profile.flip;
////}
////
////// GirlStateCoordinator
////GirlStateCoordinator::GirlStateCoordinator(Entity_t girlId) : girlId_(girlId)
////{
////	stateMachine_[Jumping] = kGirlJumpStateNode;
////	stateMachine_[Falling] = kGirlFallStateNode;
////	stateMachine_[Idle] = kGirlIdleStateNode;
////	stateMachine_[Landing] = kGirlLandStateNode;
////	stateMachine_[Walking] = kGirlWalkStateNode;
////
////	auto girl = ECS::GetEntityByID(girlId);
////	assert(girl.IsValid());
////
////	auto& sm = stateMachine_;
////
////	const auto& tf = girl.GetComponent<Transform>();
////
////	sm.context.deltas.lastPosition = tf.position;
////	
////	sm.current = Idle;
////	sm[Idle].onEnter(stateMachine_.context, girl);
////}
////
////void GirlStateCoordinator::RegisterSystems(SystemManager& sysManager)
////{
////	sysManager.RegisterSystem<EvaluationSystem>(Phase::Input, *this);
////	sysManager.RegisterSystem<IntentSystem>(Phase::Simulation, *this);
////	sysManager.RegisterSystem<AnimationSystem>(Phase::Presentation, *this);
////}
////
////void GirlStateCoordinator::UpdateStateContext(Entity& e, float dt)
////{
////	const auto& tf = e.GetComponent<Transform>();
////
////	auto& ctxDeltas = stateMachine_.context.deltas;
////
////	ctxDeltas.deltaAnimMove.x += std::abs(tf.position.x - ctxDeltas.lastPosition.x);
////	ctxDeltas.deltaAnimMove.y += std::abs(tf.position.y - ctxDeltas.lastPosition.y);
////
////	ctxDeltas.deltaAnimTime += dt;
////	ctxDeltas.deltaFrameTime = dt;
////	
////	stateMachine_.context.targets.dt = dt;
////}
////
////void GirlStateCoordinator::ResolveStateContext(Entity& e)
////{
////	auto& ctx = stateMachine_.context;
////	auto& deltas = ctx.deltas;
////
////	if (e.HasComponent<NeedsAnimationUpdate>())
////	{
////		deltas.deltaAnimMove = { 0.0f, 0.0f };
////		deltas.deltaAnimTime = 0.0f;
////	}
////
////	auto [tf, intent] = e.GetComponents<Transform, GirlIntent>();
////
////	ctx.deltas.lastPosition = tf.position;
////
////	intent.attackIntent = InputState::None;
////	intent.jumpIntent = InputState::None;
////	intent.moveIntent = { 0.0f, 0.0f };
////
////	ctx.flags = 0;
////}
////
////} // test