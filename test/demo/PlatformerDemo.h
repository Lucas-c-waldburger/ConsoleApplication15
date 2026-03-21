#pragma once
#include <optional>
#include <ranges>
#include <format>
#include <bitset>
#include "platformer/Setup.h"
#include "platformer/GirlPhysicsEditor.h"
#include "platformer/GirlChecks.h"
#include "../../core/ReadOnly.h"
#include "../../systems/GuiSystem.h"
#include "platformer/states/AttackAnimDataReference.h"
#include "platformer/states/CooldownTimer.h"
#include "platformer/states/JumpQuery.h"
#include "platformer/GamepadUiOverlay.h"

namespace test {

class GirlStateUpdater : HasWriteAccessImpl<GirlStateUpdater, B2Body, B2Shape>
{
private:
	using GirlComponentSuite = TypeList<Transform, GirlState, RigidBody, Collider,
										SpriteRenderableComponent, MoveTargets,
										AnimationDeltas, GameControllerState>;

public:
	using State = GirlState::Animation;

	GirlStateUpdater(Entity_t girl, EventBus2& bus) : girl_(girl), eventBus_(&bus) {}

	void Update(float dt)
	{
		auto e = ECS::GetEntityByID(girl_);
		if (!e.IsValid())
		{
			return;
		}

		auto [tf, state, rigid, coll, rend, targets, animDeltas, gc] = 
			e.GetComponents<GirlComponentSuite>();

		UpdateMembers(state, tf, targets, animDeltas, dt);

		auto animCopy = GetAnimComponentCopy(e);

		if (jumpQuery_.JumpBuffered())
		{
			if (ShouldJump(state, animCopy))
			{
				SetGirlJumpState(animCopy, state, rigid, coll, targets);
			}
		}
		else if (IsGirlAttackFlagged(state))
		{
			if (ShouldAttack(state, animCopy))
			{
				SetGirlAttackState(animCopy, state, rigid, coll);
			}
		}
		//else if (IsGirlDashFlagged(state))
		//{
		//	if (ShouldDash(state, animCopy))
		//	{
		//		SetGirlDashState(tf, animCopy, state, rend, rigid, coll);
		//	}
		//}
		else if (IsGirlCurrentlyJumping(state))
		{
			if (IsYIncreasing(tf.position))
			{
				if (IsGirlOnGround(state))
				{
					SetGirlLandState(animCopy, state, rigid, coll);
				}
				else
				{
					SetGirlFallState(animCopy, state, rigid, coll);
				}
			}
			else if (!JumpIntentMatchesAny(state, InputState::Pressed, InputState::Held))
			{
				CutJump(rigid);
			}
		}
		else if (IsGirlCurrentlyFalling(state))
		{
			if (IsGirlOnGround(state))
			{
				if (IsThumbstickEngaged(gc))
				{
					SetGirlWalkState(animCopy, state, rigid, coll);
				}
				else
				{
					SetGirlLandState(animCopy, state, rigid, coll);
				}
			}
		}
		else if (IsGirlCurrentlyLanding(state))
		{
			if (!IsGirlOnGround(state))
			{
				SetGirlFallState(animCopy, state, rigid, coll);
			}
			else if (state.action.moveIntent.has_value())
			{
				SetGirlWalkState(animCopy, state, rigid, coll);
			}
			else if (IsGirlAtEndOfAnimationSeries(animCopy) &&
					 TimeInAnimCrossesThreshold(animDeltas.landTime,
												animDeltas.landTimeEndMod))
			{
				SetGirlIdleState(animCopy, state, rigid, coll);
			}
		}
		else if (IsGirlCurrentlyIdle(state))
		{
			if (GirlMoved(tf))
			{
				if (IsGirlOnGround(state))
				{
					SetGirlWalkState(animCopy, state, rigid, coll);
				}
				else
				{
					SetGirlFallState(animCopy, state, rigid, coll);
				}
			} 
		}
		else if (IsGirlCurrentlyWalking(state))
		{
			if (!IsGirlOnGround(state))
			{
				SetGirlFallState(animCopy, state, rigid, coll);
			}
			else if (WalkVelocityXUnderStopThreshold(rigid) &&
					 !IsThumbstickEngaged(gc))
			{
				StopWalkVelocityX(rigid);
			}

			if (!GirlMoved(tf))
			{
				SetGirlIdleState(animCopy, state, rigid, coll);
			}
		}
		else if (IsGirlCurrentlyAttacking(state))
		{
			if (IsGirlAtEndOfAnimationSeries(animCopy) &&				
				TimeInAnimCrossesThreshold(
				attackAnimDataRef_.GetAttackAnimChangeTime(animDeltas)))
			{
				if (IsGirlOnGround(state))
				{
					SetGirlIdleState(animCopy, state, rigid, coll);
				}
				else
				{
					SetGirlFallState(animCopy, state, rigid, coll);
				}
			}
		}
		else if (IsGirlCurrentlyDashing(state))
		{
			if (GirlCompletedDashAnimation(state, animCopy))
			{
				if (IsGirlOnGround(state))
				{
					SetGirlIdleState(animCopy, state, rigid, coll);
				}
				else
				{
					SetGirlFallState(animCopy, state, rigid, coll);
				}
			}
		}

		HandleAxisMoveIntent(rigid, state, targets, dt);

		UpdateAnimations(tf, animCopy, state, animDeltas);
		UpdateSpriteFacingSide(tf, rend, animCopy, state);

		if (SpriteChanged(animCopy, e))
		{
			UpdateForSpriteChange(animCopy, e);
		}

		Cleanup(tf, state);
	}


private:
	bool TimeInAnimCrossesThreshold(float threshold, float mod = 0.0f) const
	{
		return timeInCurrentAnimFrame_ >= threshold + mod;
	}
	bool DistXInAnimCrossesThreshold(float threshold, float mod = 0.0f) const
	{
		return distanceInCurrentAnimFrame_.x >= threshold + mod;
	}
	bool DistYInAnimCrossesThreshold(float threshold, float mod = 0.0f) const
	{
		return distanceInCurrentAnimFrame_.y >= threshold + mod;
	}

	void PushStateChangeEvent(const GirlState& state, GirlState::Animation newState)
	{
		if (eventBus_)
		{
			eventBus_->PushAndDispatchEvents(GirlStateChangeEvent{
				.lastState = state.animation,
				.newState = newState
			});
		}
	}

	void SetGirlIdleState(SpriteAnimationComponent& anim, GirlState& state, RigidBody& rigid,
						  Collider& collider) 
	{
		ExitCurrentState(state, rigid, collider);

		anim.spriteSeriesName = "girl_idle";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Idle);

		state.animation = GirlState::Animation::Idle;
	}
	void SetGirlJumpState(SpriteAnimationComponent& anim, GirlState& state,
						  RigidBody& rigid, Collider& collider, const MoveTargets& targets)
	{
		ExitCurrentState(state, rigid, collider);

		anim.spriteSeriesName = "girl_jump";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Jumping);

		state.animation = GirlState::Animation::Jumping;

		const float jumpImpulseY = ComputeJumpImpulseY(rigid, targets.jumpVelY);

		rigid.forceRequests.impulses.emplace_back(
			Force{ .value = { 0.0f, -jumpImpulseY } });
	}
	void SetGirlWalkState(SpriteAnimationComponent& anim, GirlState& state, 
						  RigidBody& rigid, Collider& collider)
	{
		ExitCurrentState(state, rigid, collider);

		anim.spriteSeriesName = "girl_walk";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Walking);

		state.animation = GirlState::Animation::Walking;
	}
	void SetGirlLandState(SpriteAnimationComponent& anim, GirlState& state,
						  RigidBody& rigid, Collider& collider)
	{
		ExitCurrentState(state, rigid, collider);

		anim.spriteSeriesName = "girl_land";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Landing);

		state.animation = GirlState::Animation::Landing;

		//GetWriteAccess(rigid.body).SetLinearVelocity({0.0f, 0.0f});
		//GetWriteAccess(collider.shape).SetFriction(kGirlColliderLandingFriction);
	}
	void ExitGirlLandState(Collider& collider)
	{
		//GetWriteAccess(collider.shape).SetFriction(kGirlColliderFriction);
	}

	void SetGirlFallState(SpriteAnimationComponent& anim, GirlState& state,
						  RigidBody& rigid, Collider& collider)
	{
		ExitCurrentState(state, rigid, collider);

		anim.spriteSeriesName = "girl_fall";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Falling);

		state.animation = GirlState::Animation::Falling;

		GetWriteAccess(rigid.body).SetGravityScale(2.5f);
	}
	void ExitGirlFallState(RigidBody& rigid)
	{
		GetWriteAccess(rigid.body).SetGravityScale(1.0f);
	}

	void SetGirlAttackState(SpriteAnimationComponent& anim, GirlState& state,
							RigidBody& rigid, Collider& collider)
	{
		ExitCurrentState(state, rigid, collider);

		attackAnimDataRef_.MarkNewAttack();

		anim.spriteSeriesName = attackAnimDataRef_.GetAttackAnimSeriesName();
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Attacking);

		state.animation = GirlState::Animation::Attacking;

		//if (IsGirlOnGround(state))
		//{
		//	GetWriteAccess(collider.shape).SetFriction(kGirlColliderLandingFriction);
		//}
	}
	void ExitGirlAttackState(Collider& collider)
	{
		GetWriteAccess(collider.shape).SetFriction(kGirlColliderFriction);

		attackAnimDataRef_.MarkAttackEnd();
	}

	bool GirlMoved(const Transform& tf) const
	{
		return !(EqualsWithTolerance(tf.position.x, lastPosition_.x) &&
				 EqualsWithTolerance(tf.position.y, lastPosition_.y));
	}

	void CutJump(RigidBody& rigid)
	{
		auto& body = GetWriteAccess(rigid.body);

		auto vel = body.GetLinearVelocity();
		vel.y *= 0.7f;

		body.SetLinearVelocity(vel);
	}

	SDL_FPoint GetDashImpulse(const GirlState& state, RigidBody& rigid,
						      const SpriteRenderableComponent& rend) const
	{
		SDL_FPoint dashImpulse = { 0.0f, 0.0f };
		const auto& moveIntent = state.action.moveIntent;

		if (moveIntent.has_value())
		{
			if (!EqualsWithTolerance(moveIntent->x, 0.0f))
			{
				dashImpulse.x = moveIntent->x < 0 ? -kGirlDashImpulseX 
												  : kGirlDashImpulseX;
			}
			if (!EqualsWithTolerance(moveIntent->y, 0.0f))
			{
				dashImpulse.y = moveIntent->y < 0 ? -kGirlDashImpulseX / 3.0f 
												  : kGirlDashImpulseX / 3.0f;
			}
		}
		else
		{
			dashImpulse.x = (rend.profile.flip == SDL_FLIP_HORIZONTAL) ? -kGirlDashImpulseX
																	   : kGirlDashImpulseX;
		}

		return ComputeDashImpulse(rigid, dashImpulse);
	}

	void SetGirlDashState(Transform& tf, SpriteAnimationComponent& anim, GirlState& state,
						  SpriteRenderableComponent& rend,
						  RigidBody& rigid, Collider& collider)
	{
		ExitCurrentState(state, rigid, collider);

		anim.spriteSeriesName = "girl_dash";
		anim.index.current = 0;

		PushStateChangeEvent(state, State::Dashing);

		state.animation = State::Dashing;

		GetWriteAccess(collider.shape).SetFriction(0.0f);

		const SDL_FPoint dashImpulse = GetDashImpulse(state, rigid, rend);

		rigid.forceRequests.impulses.emplace_back(Force{
			.value = dashImpulse
		});
	}
	void ExitGirlDashState(RigidBody& rigid, Collider& collider)
	{
		auto& body = GetWriteAccess(rigid.body);

		auto vel = body.GetLinearVelocity();
		vel.x *= 0.2f;

		body.SetLinearVelocity(vel);

		GetWriteAccess(collider.shape).SetFriction(kGirlColliderFriction);

		dashCooldown_.Reset();
	}

	void StopWalkVelocityX(RigidBody& rigid)
	{
		auto& body = GetWriteAccess(rigid.body);

		body.SetLinearVelocity({
			0.0f,
			body.GetLinearVelocity().y
		});
	}

	bool IsYIncreasing(SDL_FPoint currentPos) const
	{
		return currentPos.y > lastPosition_.y;
	}

	bool GirlCompletedDashAnimation(const GirlState& state, 
									const SpriteAnimationComponent& anim) const
	{
		return IsGirlAtEndOfAnimationSeries(anim) &&
			  //((distanceInCurrentAnimFrame_.x > kGirlDashAnimXDeltaDuration ||
			  // distanceInCurrentAnimFrame_.y > kGirlDashAnimYDeltaDuration) ||
			   TimeInAnimCrossesThreshold(kGirlDashAnimEnforcedTime);
	}

	bool ShouldDash(const GirlState& state, const SpriteAnimationComponent& anim) const
	{
		return dashCooldown_.Ready() &&
			GirlNotInState(state, State::Dashing) &&
			GirlNotInState(state, State::Attacking).Or(IsGirlAtEndOfAnimationSeries(anim, 1));
		//return !IsGirlCurrentlyDashing(state) && dashCooldown_.Ready();
	}

	void ExitCurrentState(GirlState& state, RigidBody& rigid, Collider& collider)
	{
		switch (state.animation)
		{
		case State::Falling:
			ExitGirlFallState(rigid); break;
		case State::Landing:
			ExitGirlLandState(collider); break;
		case State::Attacking:
			ExitGirlAttackState(collider); break;
		case State::Dashing:
			ExitGirlDashState(rigid, collider); break;
		default:
			break;
		}
	}

	void UpdateAnimations(const Transform& tf, SpriteAnimationComponent& anim,
						  GirlState& state, const AnimationDeltas& deltas) const
	{
		switch (state.animation)
		{
		case State::Idle:
			assert(anim.spriteSeriesName == "girl_idle");
			if (TimeInAnimCrossesThreshold(deltas.idleTime))
			{
				++anim.index;
			}
			break;
		case State::Walking:
			assert(anim.spriteSeriesName == "girl_walk");
			if (DistXInAnimCrossesThreshold(deltas.walkDeltaX))
			{
				++anim.index;
			}
			break;
		case State::Jumping:
			assert(anim.spriteSeriesName == "girl_jump");

			if (DistYInAnimCrossesThreshold(jumpQuery_.GetJumpAnimDeltaY()) &&
				!IsGirlAtEndOfAnimationSeries(anim))
			{
				++anim.index;
			}
			break;
		case State::Falling:
			assert(anim.spriteSeriesName == "girl_fall");
			if (DistYInAnimCrossesThreshold(deltas.fallDeltaY) &&
				!IsGirlAtEndOfAnimationSeries(anim))
			{
				++anim.index;
			}
			break;
		case State::Landing:
			assert(anim.spriteSeriesName == "girl_land");
			if (TimeInAnimCrossesThreshold(deltas.landTime) &&
				!IsGirlAtEndOfAnimationSeries(anim))
			{
				++anim.index;
			}
			break;
		case State::Attacking:
			assert(anim.spriteSeriesName == attackAnimDataRef_.GetAttackAnimSeriesName());
			if (TimeInAnimCrossesThreshold(attackAnimDataRef_.GetAttackAnimChangeTime(deltas)) &&
				!IsGirlAtEndOfAnimationSeries(anim))
			{
				++anim.index;
			}
			break;
		case State::Dashing:
			assert(anim.spriteSeriesName == "girl_dash");
			if (TimeInAnimCrossesThreshold(kGirlDashAnimChangeTime) &&
				!IsGirlAtEndOfAnimationSeries(anim))
			{
				++anim.index;
			}
			break;
		default:
			break;
		}
	}

	void UpdateSpriteFacingSide(Transform& tf, SpriteRenderableComponent& rend,
								const SpriteAnimationComponent& anim, GirlState& state) const
	{
		if (GirlInState(state, State::Landing))
		{
			return;
		}
		if (GirlInState(state, State::Attacking)
			.And(!IsGirlAtEndOfAnimationSeries(anim, 1)))
		{
			return;
		}

		if (state.action.moveIntent.has_value())
		{
			rend.profile.flip = state.action.moveIntent->x < 0.0f ? SDL_FLIP_HORIZONTAL :
								state.action.moveIntent->x > 0.0f ? SDL_FLIP_NONE :
								rend.profile.flip;
		}
		else
		{ 
			rend.profile.flip = (tf.position.x < lastPosition_.x) ? SDL_FLIP_HORIZONTAL :
								(tf.position.x > lastPosition_.x) ? SDL_FLIP_NONE :
								rend.profile.flip;
		}
	}

	void UpdateMembers(const GirlState& state, const Transform& tf, 
					   MoveTargets& targets, const AnimationDeltas& animDeltas, float dt)
	{
		distanceInCurrentAnimFrame_.x +=
			std::abs(tf.position.x - lastPosition_.x);
		distanceInCurrentAnimFrame_.y +=
			std::abs(tf.position.y - lastPosition_.y);

		timeInCurrentAnimFrame_ += dt;
		targets.dt = dt;

		attackAnimDataRef_.Update(dt);
		dashCooldown_.Update(dt);
		jumpQuery_.Update(state, animDeltas);
	}

	static void HandleAxisMoveIntent(RigidBody& rigid, GirlState& state, 
									 MoveTargets& targets, float dt)
	{
		if (!state.action.moveIntent.has_value())
		{
			return;
		}
		if (IsGirlCurrentlyAttacking(state) || IsGirlCurrentlyDashing(state))
		{
			return;
		}

		const float normedX = state.action.moveIntent->x /
							  static_cast<float>(GameController::kAxisMax);

		targets.targetVelX = normedX * targets.maxSpeed;

		float moveImpulseX = ComputeMoveImpulseX(rigid, state, targets, targets.targetVelX, dt);
		//if (IsGirlCurrentlyLanding(state))
		//{
		//	moveImpulseX /= 2.0f;
		//}

		rigid.forceRequests.impulses.emplace_back(
			Force{ .value = { moveImpulseX, 0.0f } });
	}

	static SpriteAnimationComponent GetAnimComponentCopy(const Entity& girl)
	{
		return girl.GetComponent<SpriteAnimationComponent>();
	}

	void UpdateForSpriteChange(SpriteAnimationComponent& animCopy,
							   Entity& girl)
	{
		girl.GetComponent<SpriteAnimationComponent>() = std::move(animCopy);
		timeInCurrentAnimFrame_ = 0.0f;
		distanceInCurrentAnimFrame_ = { 0.0f, 0.0f };
	}

	void Cleanup(const Transform& tf, GirlState& state)
	{
		lastPosition_ = tf.position;
		lastState_ = state;

		state.action.Reset();
	}

	Entity_t girl_;
	SDL_FPoint lastPosition_ = { 0.0f, 0.0f };
	GirlState lastState_;
	float timeInCurrentAnimFrame_ = 0.0f;
	SDL_FPoint distanceInCurrentAnimFrame_ = { 0.0f, 0.0f };
	AttackAnimDataReference attackAnimDataRef_;
	JumpQuery jumpQuery_;
	EventBus2* eventBus_ = nullptr;
	CooldownTimer dashCooldown_{ .duration = kGirlDashCooldownTime };
};


static Result<Void> RunPlatformerDemo(SceneFixture::SharedPtr& fixture)
{
	TRY(SetUpEnvironment(fixture));

	const auto [winCenterX, winCenterY] = 
		SDLite::Window().GetLocalCenter<SDL_FPoint>();

	TRY(MakeGirlEntity(fixture, {winCenterX, winCenterY - 60.0f}), girlEnt);

	fixture->RegisterSystem<GirlStateUpdater>(Phase::Input, 
		girlEnt.GetID(), fixture->GetEventBus());
	//fixture->RegisterSystem<GirlStateCoordinator>(Phase::Input,
	//	girlEnt.GetID(), fixture->GetEventBus());

	TRY(SetUpGirlStateReporter(girlEnt, fixture));
	TRY(ThumbstickUiDraw::CreateAndConnect(girlEnt, fixture));
	TRY(ControllerButtonUiDraw::CreateAndConnect(girlEnt, fixture));

#if IMGUI_ENABLED

	assert(fixture->IsSystemRegistered<GuiSystem>());

	auto& guiSys = fixture->GetSystem<GuiSystem>();

	TRY(GirlPhysicsEditor::Init(guiSys, girlEnt));

#endif

	return fixture->RunGameLoop();
}


















} // test