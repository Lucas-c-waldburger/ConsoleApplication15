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
#include "../ui/Editor.h"

namespace test {

class GirlStateUpdater : HasWriteAccessImpl<GirlStateUpdater, B2Body, B2Shape>
{
private:
	using GirlComponentSuite = TypeList<Transform, GirlState, RigidBody, Collider,
										SpriteRenderableComponent, MoveTargets,
										AnimationDeltas, GameControllerState>;

public:
	using State = GirlState::Animation;

	GirlStateUpdater(Entity_t girl, Entity_t sword, EventBus& bus) : 
		girl_(girl), sword_(sword), eventBus_(&bus) {}
	GirlStateUpdater(Entity_t girl, Entity_t sword, EventBus& bus, LegIron&& legIron) :
		girl_(girl), sword_(sword), eventBus_(&bus), legIron_(std::move(legIron)) {}

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

		if (jumpQuery_.JumpBuffered() && ShouldJump(state, animCopy, coll))
		{
			SetGirlJumpState(animCopy, state, rigid, coll, targets);
		}
		else if (IsGirlAttackFlagged(state) && ShouldAttack(state, animCopy))
		{
			SetGirlAttackState(animCopy, state, rigid, coll);
		}
		else if (IsGirlCurrentlyJumping(state))
		{
			if (IsYIncreasing(tf.position))
			{
				if (IsGirlOnGround(state, coll))
				{
					SetGirlLandState(animCopy, state, rigid, coll);
				}
				else
				{
					SetGirlFallState(animCopy, state, rigid, coll);
				}
			}
			else if (JumpIntentMatchesAny(state, InputState::Released))
			{
				CutJump(rigid);
			}
		}
		else if (IsGirlCurrentlyFalling(state))
		{
			if (IsGirlOnGround(state, coll))
			{
				SetGirlIdleState(animCopy, state, rigid, coll);
			}
		}
		else if (IsGirlCurrentlyLanding(state))
		{
			if (!IsGirlOnGround(state, coll))
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
			if (!IsGirlOnGround(state, coll))
			{
				SetGirlFallState(animCopy, state, rigid, coll);
			}
			else if (IsThumbstickEngaged(state))
			{
				SetGirlWalkState(animCopy, state, rigid, coll);
			} 
			else if (ShouldSheathSword())
			{
				SetGirlSheathState(animCopy, state, rigid, coll);
			}

			if (ShouldCarry(state))
			{
				SetGirlCarrySubState(state, true, animCopy, rigid);
			}
		}
		else if (IsGirlCurrentlyWalking(state))
		{
			if (!IsGirlOnGround(state, coll))
			{
				SetGirlFallState(animCopy, state, rigid, coll);
			}
			else if (!IsThumbstickEngaged(state))
			{
				StopWalkVelocityX(rigid);
			}

			if (!IsThumbstickEngaged(state))
			{
				SetGirlIdleState(animCopy, state, rigid, coll);
			}

			if (ShouldCarry(state))
			{
				SetGirlCarrySubState(state, true, animCopy, rigid);
			}
		}
		else if (IsGirlCurrentlyAttacking(state))
		{
			if (IsGirlAtEndOfAnimationSeries(animCopy) &&				
				TimeInAnimCrossesThreshold(
				attackAnimDataRef_.GetAttackAnimChangeTime(animDeltas)))
			{
				if (IsGirlOnGround(state, coll))
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
				if (IsGirlOnGround(state, coll))
				{
					SetGirlIdleState(animCopy, state, rigid, coll);
				}
				else
				{
					SetGirlFallState(animCopy, state, rigid, coll);
				}
			}
		}
		else if (IsGirlCurrentlySheathingSword(state))
		{
			if (!IsGirlOnGround(state, coll))
			{
				SetGirlFallState(animCopy, state, rigid, coll);
			}
			else if (IsThumbstickEngaged(state))
			{
				SetGirlWalkState(animCopy, state, rigid, coll);
			}
			else if (IsGirlAtEndOfAnimationSeries(animCopy) &&
					 TimeInAnimCrossesThreshold(animDeltas.sheathTime)) 
			{
				SetGirlIdleState(animCopy, state, rigid, coll);
			}
		}

		// SUB STATES
		if (IsGirlAlsoCarryingBall(state))
		{
			if (ShouldDrop(state))
			{
				SetGirlCarrySubState(state, false, animCopy, rigid);
			}
			else if (animCopy.spriteSeriesName != "girl_push")
			{
				animCopy.spriteSeriesName = "girl_push";
			}
		}

		HandleAxisMoveIntent(rigid, state, targets, dt);

		UpdateAnimations(tf, animCopy, state, animDeltas);
		UpdateSpriteFacingSide(tf, rend, animCopy, state);

		UpdateSword(state, animCopy, rend, tf);
		UpdateLegIron(state, tf, rend);

		if (SpriteChanged(animCopy, e))
		{
			UpdateForSpriteChange(animCopy, e);
		}

		Cleanup(tf, state);
	}

	Entity GetLegIronBallEntity() { return legIron_.GetBallEntity(); }

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

		if (isSwordOut_)
		{
			anim.spriteSeriesName = "girl_idle_s";
			sheathIdleCooldown_.Reset();
		}
		else
		{
			anim.spriteSeriesName = "girl_idle";
		}
		
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Idle);

		state.animation = GirlState::Animation::Idle;
	}
	void SetGirlJumpState(SpriteAnimationComponent& anim, GirlState& state,
						  RigidBody& rigid, Collider& collider, const MoveTargets& targets)
	{
		ExitCurrentState(state, rigid, collider);

		anim.spriteSeriesName = (isSwordOut_) ? "girl_jump_s" : "girl_jump";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Jumping);

		state.animation = GirlState::Animation::Jumping;

		const float jumpImpulseY = ComputeJumpImpulseY(rigid, targets.jumpVelY);

		rigid.forceRequests.impulses.emplace_back(
			Force{ .value = { 0.0f, -jumpImpulseY } });

		//GetWriteAccess(rigid.body).SetGravityScale(1.8f);
		GetWriteAccess(rigid.body).SetGravityScale(kGirlJumpGravityScale);
	}
	void SetGirlWalkState(SpriteAnimationComponent& anim, GirlState& state, 
						  RigidBody& rigid, Collider& collider)
	{
		ExitCurrentState(state, rigid, collider);

		anim.spriteSeriesName = (isSwordOut_) ? "girl_walk_s" : "girl_walk";
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

		anim.spriteSeriesName = (isSwordOut_) ? "girl_fall_s" : "girl_fall";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Falling);

		state.animation = GirlState::Animation::Falling;

		//GetWriteAccess(rigid.body).SetGravityScale(2.5f);
		GetWriteAccess(rigid.body).SetGravityScale(kGirlJumpGravityScale);
	}
	void ExitGirlFallState(RigidBody& rigid)
	{
		GetWriteAccess(rigid.body).SetGravityScale(kGirlNormalGravityScale);
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

		isSwordOut_ = true;
	}
	void ExitGirlAttackState(Collider& collider)
	{
		GetWriteAccess(collider.shape).SetFriction(kGirlColliderFriction);

		attackAnimDataRef_.MarkAttackEnd();

		if (auto* swordRend = GetSwordRenderable())
		{
			SetSwordActive(*swordRend, false);
		}
	}

	void SetGirlSheathState(SpriteAnimationComponent& anim, GirlState& state,
							RigidBody& rigid, Collider& collider)
	{
		ExitCurrentState(state, rigid, collider);

		anim.spriteSeriesName = "girl_sheath";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Sheathing);

		state.animation = GirlState::Animation::Sheathing;

		isSwordOut_ = false;
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
		vel.y *= 0.5f;

		body.SetLinearVelocity(vel);
		//body.SetGravityScale(2.5f);
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

		auto vel = body.GetLinearVelocity();
		vel.x *= 0.2f;

		body.SetLinearVelocity(vel);
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

	bool ShouldSheathSword() const
	{
		return isSwordOut_ && sheathIdleCooldown_.Ready();
	}

	SpriteRenderableComponent* GetSwordRenderable()
	{
		auto sw = ECS::GetEntityByID(sword_);
		if (!sw.IsValid())
		{
			return nullptr;
		}

		assert(sw.HasComponent<SpriteRenderableComponent>());

		return &sw.GetComponent<SpriteRenderableComponent>();
	}


	static bool IsSwordActive(const SpriteRenderableComponent& rend) 
	{
		return rend.profile.debugDraw.collider.on;
	}

	static bool InSwordAnimationWindow(const SpriteAnimationComponent& anim)
	{
		const auto [minIdx, maxIdx] = kGirlAttackAnimSwordActiveIdxRange;
		return anim.index.current >= minIdx && anim.index.current <= maxIdx;
	}

	static bool IsGirlFacingRight(const SpriteRenderableComponent& rend)
	{
		return rend.profile.flip == SDL_FLIP_NONE;
	}

	static void SetSwordActive(SpriteRenderableComponent& rend, bool active)
	{
		rend.profile.debugDraw.collider.on = active;
	}

	static bool ShouldCarry(const GirlState& state)
	{
		return ((state.substate & GirlState::SubState::CarryingBall) == 0) &&
			     state.collidingCategories[ObjectCategory::PlayerLegIron] > 0 &&
			     state.action.carryIntent == InputState::Pressed;
	}

	static bool ShouldDrop(const GirlState& state)
	{
		return (state.action.carryIntent & (InputState::Pressed | InputState::Held)) == 0;
	}

	//void SetGirlCarryState(SpriteAnimationComponent& anim, GirlState& state,
	//					           RigidBody& rigid, Collider& collider)
	//{
	//	ExitCurrentState(state, rigid, collider);

	//	anim.spriteSeriesName = "girl_push";
	//	anim.index.current = 0;

	//	PushStateChangeEvent(state, GirlState::Animation::Carrying);

	//	state.animation = GirlState::Animation::Carrying;
	//}
	void SetGirlCarrySubState(GirlState& state, bool on, SpriteAnimationComponent& anim,
							  RigidBody& girlRigid)
	{
		if (on)
		{
			state.substate |= GirlState::SubState::CarryingBall;
			anim.spriteSeriesName = "girl_push";
			anim.index.current = 0;
			isSwordOut_ = false;
		}
		else
		{
			state.substate &= ~(GirlState::SubState::CarryingBall);
			anim.spriteSeriesName = GetSpriteSeriesNameForState(state);
			anim.index.current = 0;
		}

		auto ballE = legIron_.GetBallEntity();

		if (!ballE.IsValid() || !ballE.HasComponent<RigidBody>())
		{
			return;
		}

		auto& ballBody = GetWriteAccess(ballE.GetComponent<RigidBody>().body);

		ballBody.SetGravityScale(on ? 0.0f : 2.5f);

		if (on)
		{
			ballBody.SetAngularDamping(20.0f);
		}
		else
		{
			auto girlBody = GetWriteAccess(girlRigid.body);
			if (!girlBody.IsValid())
			{
				LOG_DEBUG("Girl body was invalid");
				return;
			}
			
			auto velToApply = girlBody.GetLinearVelocity() * 1.3f;
			if (state.action.moveIntent.has_value())
			{
				auto normed = std::abs(*state.action.moveIntent / 
					static_cast<float>(GameController::kAxisMax));

				velToApply *= normed * 2.0f;
				//velToApply.y *= 1.03f;
			}

			ballBody.SetLinearVelocity(velToApply);
		}
	}


	void UpdateLegIron(const GirlState& state, const Transform& tf, const SpriteRenderableComponent& rend)
	{
		auto ballE = legIron_.GetBallEntity();

		if (!ballE.IsValid() || !ballE.HasComponent<RigidBody>())
		{
			return;
		}

		auto& ballBody = GetWriteAccess(ballE.GetComponent<RigidBody>().body);

		std::optional<SDL_FPoint> newBallPos;

		if (!IsGirlAlsoCarryingBall(state))
		{
			static constexpr float kRetractAmount = 0.05f;
			const auto currentJointLen = legIron_.GetJointRestLength();

			if (state.action.retractIntent & (InputState::Pressed | InputState::Held))
			{
				if (currentJointLen - kRetractAmount > 0.0f)
				{
					legIron_.SetJointRestLength(currentJointLen - kRetractAmount);				
				}

			}
			else
			{
				if (currentJointLen + kRetractAmount < legIron_.GetDefaultJointLength())
				{
					legIron_.SetJointRestLength(currentJointLen + kRetractAmount);
				}
			}

			if (state.action.ballFreezeIntent & (InputState::Pressed | InputState::Held))
			{
				if (CanBallBeFrozen(ballE))
				{
					if (!ballLockPosition_.has_value())
					{
						ballLockPosition_ = ballBody.GetPosition();
					}
					newBallPos = *ballLockPosition_;

					ballBody.SetLinearVelocity({ 0.0f, 0.0f });
					ballBody.SetAngularVelocity(0.0f);
					ballBody.SetGravityScale(0.0f);
				}
			}
			else
			{
				ballLockPosition_.reset();
				ballBody.SetGravityScale(1.0f);
			}
		}
		else
		{
			static constexpr float ballCarryOffsetX = 40.0f;
			static constexpr float ballCarryOffsetY = -40.0f;

			newBallPos = {
				tf.position.x + (IsGirlFacingRight(rend) ? ballCarryOffsetX : -ballCarryOffsetX),
				tf.position.y + ballCarryOffsetY
			};
		}

		if (newBallPos.has_value())
		{
			ballBody.SetPosition(*newBallPos);
		}
		//ballBody.SetLinearVelocity({ 0.0f, 0.0f });
		//ballBody.SetAngularVelocity(0.0f);
	}

	void UpdateSword(const GirlState& state, const SpriteAnimationComponent& anim, 
					 const SpriteRenderableComponent& rend, const Transform& tf)
	{
		auto sw = ECS::GetEntityByID(sword_);
		if (!sw.IsValid())
		{
			return;
		}

		assert((sw.HasComponents<SpriteRenderableComponent, RigidBody>()));
		auto [swordRend, swordRigid] = sw.GetComponents<SpriteRenderableComponent, RigidBody>();
		
		auto& swordBody = GetWriteAccess(swordRigid.body);

		float xOffset = kGirlIdleHitboxDimensions.w * kGirlSpriteScale.x;

		swordBody.SetPosition({
			tf.position.x + (IsGirlFacingRight(rend) ? xOffset : -xOffset),
			tf.position.y
		});

		if (state.animation != State::Attacking)
		{
			return;
		}

		if (!IsSwordActive(swordRend))
		{
			if (InSwordAnimationWindow(anim))
			{
				SetSwordActive(swordRend, true);		
			}
		}
		else
		{
			if (!InSwordAnimationWindow(anim))
			{
				SetSwordActive(swordRend, false);
				shapesHitBySword_.clear();
			} 
		}

		// apply impulse on colliding shapes
		if (IsSwordActive(swordRend))
		{
			assert((sw.HasComponents<Collider, CollisionCache>()));
			auto [collider, collisionCache] = 
				sw.GetComponents<Collider, CollisionCache>();

			if (collisionCache.data.empty())
			{
				return;
			}

			auto swordShape = GetWriteAccess(collider.shape);
			assert(swordShape.IsValid());

			const auto swordPos = swordBody.GetPosition();

			const auto axis = state.action.moveIntent.value_or(SDL_FPoint{ 0.0f, 0.0f });
			auto normed = GetAxisNormed(axis);

			float hitImpulseX = IsGirlFacingRight(rend)
				? kSwordHitImpulse
				: -kSwordHitImpulse;
			float hitImpulseY = normed.y * kSwordHitImpulse * 2.0f;

			for (const auto& [otherShape, otherEntId] : collisionCache.data)
			{
				if (shapesHitBySword_.contains(otherShape))
				{
					continue;
				}

				auto otherEnt = FindOwningBodyEntity(CollisionData{
					.entity = otherEntId,
					.shapeHandle = otherShape
				});
				if (!otherEnt.IsValid())
				{
					continue;
				}

				assert(otherEnt.HasComponent<RigidBody>());
				auto& otherRigid = otherEnt.GetComponent<RigidBody>();

				otherRigid.forceRequests.impulses.emplace_back(
					Force{ .value = { hitImpulseX, hitImpulseY } }
				);

				shapesHitBySword_.insert(otherShape);
			}
		}
	}

	std::string_view GetSpriteSeriesNameForState(const GirlState& state)
	{
		const bool carryingBall = IsGirlAlsoCarryingBall(state);
		switch (state.animation)
		{
		case State::Idle:
			return (carryingBall) ? "girl_push" : (isSwordOut_) ? "girl_idle_s" : "girl_idle";
		case State::Walking:
			return (carryingBall) ? "girl_push" : (isSwordOut_) ? "girl_walk_s" : "girl_walk";
		case State::Attacking:
			return attackAnimDataRef_.GetAttackAnimSeriesName();
		case State::Jumping:
			return (isSwordOut_ && !carryingBall) ? "girl_jump_s" : "girl_jump";
		case State::Landing:
			return "girl_land";
		case State::Falling:
			return (carryingBall) ? "girl_push" : "girl_fall";
		case State::Sheathing:
			return "girl_sheath";
		default:
			return "girl_idle";
		}
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
			if (!IsGirlAlsoCarryingBall(state) && TimeInAnimCrossesThreshold(deltas.idleTime))
			{
				++anim.index;
			}
			break;
		case State::Walking:
			if (IsGirlAlsoCarryingBall(state))
			{
				if (DistXInAnimCrossesThreshold(deltas.carryDeltaX))
				{
					++anim.index;
				}
			}
			else if (DistXInAnimCrossesThreshold(deltas.walkDeltaX))
			{
				++anim.index;
			}
			break;
		case State::Jumping:
			if (DistYInAnimCrossesThreshold(jumpQuery_.GetJumpAnimDeltaY()) &&
				!IsGirlAtEndOfAnimationSeries(anim))
			{
				++anim.index;
			}
			break;
		case State::Falling:
			if (DistYInAnimCrossesThreshold(deltas.fallDeltaY) &&
				!IsGirlAtEndOfAnimationSeries(anim))
			{
				++anim.index;
			}
			break;
		case State::Landing:
			if (TimeInAnimCrossesThreshold(deltas.landTime) &&
				!IsGirlAtEndOfAnimationSeries(anim))
			{
				++anim.index;
			}
			break;
		case State::Attacking:
			if (TimeInAnimCrossesThreshold(attackAnimDataRef_.GetAttackAnimChangeTime(deltas)) &&
				!IsGirlAtEndOfAnimationSeries(anim))
			{
				++anim.index;
			}
			break;
		case State::Dashing:
			if (TimeInAnimCrossesThreshold(kGirlDashAnimChangeTime) &&
				!IsGirlAtEndOfAnimationSeries(anim))
			{
				++anim.index;
			}
			break;
		case State::Sheathing:
			if (TimeInAnimCrossesThreshold(deltas.sheathTime) &&
				!IsGirlAtEndOfAnimationSeries(anim))
			{
				++anim.index;
			}
		case State::Carrying:
			if (DistXInAnimCrossesThreshold(deltas.walkDeltaX))
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
		sheathIdleCooldown_.Update(dt);
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
	Entity_t sword_;
	SDL_FPoint lastPosition_ = { 0.0f, 0.0f };
	GirlState lastState_;
	float timeInCurrentAnimFrame_ = 0.0f;
	SDL_FPoint distanceInCurrentAnimFrame_ = { 0.0f, 0.0f };
	AttackAnimDataReference attackAnimDataRef_;
	JumpQuery jumpQuery_;
	EventBus* eventBus_ = nullptr;
	CooldownTimer dashCooldown_{ .duration = kGirlDashCooldownTime };
	CooldownTimer sheathIdleCooldown_{ .duration = kGirlSheathSwordIdleTriggerTime };
	bool isSwordOut_ = false;
	std::unordered_set<Handle<B2Shape>> shapesHitBySword_;
	LegIron legIron_;
	std::optional<SDL_FPoint> ballLockPosition_;
};

static Result<Void> RunPlatformerDemo(SceneFixture::SharedPtr& fixture)
{
	TRY(SetUpEnvironment(fixture));

	const auto [winCenterX, winCenterY] = 
		SDLite::Window().GetLocalCenter<SDL_FPoint>();

	const SDL_FPoint girlStartPos = { winCenterX, winCenterY - 60.0f };

	TRY(MakeGirlEntity(fixture, girlStartPos), girlEnt);
	TRY(MakeSwordEntity(fixture, girlEnt), swordEnt);
	//TRY(MakeCrateEntity(fixture, { girlStartPos.x + 60.0f, girlStartPos.y}), crateEnt);
	TRY(MakeLegIronEntity(fixture, girlEnt, girlStartPos), legIron);

	auto& updater = fixture->RegisterSystem<GirlStateUpdater>(Phase::Input, 
		girlEnt.GetID(), swordEnt.GetID(), fixture->GetEventBus(), std::move(legIron));

	TRY(SetUpGirlStateReporter(girlEnt, fixture));
	TRY(ThumbstickUiDraw::CreateAndConnect(girlEnt, fixture));
	TRY(ControllerButtonUiDraw::CreateAndConnect(girlEnt, fixture));

#if IMGUI_ENABLED

	assert(fixture->IsSystemRegistered<GuiSystem>());

	TRY(ui::Editor::Init(fixture));
	//TRY(ui::EntityInspector2::Setup(fixture));

	//auto& guiSys = fixture->GetSystem<GuiSystem>();

	//EntityMap entities{};
	//entities["girl"] = girlEnt;
	//entities["sword"] = swordEnt;
	//entities["legIron"] = updater.GetLegIronBallEntity();
	////entities["crate"] = crateEnt;

	//TRY(GirlPhysicsEditor::Init(guiSys, entities));

#endif

	return fixture->RunGameLoop();
}


















} // test