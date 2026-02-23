#pragma once
#include <optional>
#include <ranges>
#include <format>
#include <bitset>
#include "platformer/Setup.h"
#include "platformer/GirlPhysicsEditor.h"
#include "../../systems/GuiSystem.h"

namespace test {

class GirlStateUpdater
{
private:
	using GirlComponentSuite = TypeList<Transform, GirlState, RigidBody, Collider,
										SpriteRenderableComponent, MoveTargets,
										AnimationDeltas, GameControllerState>;

public:
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

		UpdateMemberDeltas(tf, targets, dt);

		HandleAxisMoveIntent(rigid, state, targets);

		//const auto& constE = e;
		//auto animCopy = constE.GetComponent<SpriteAnimationComponent>();
		auto animCopy = GetAnimComponentCopy(e);

		if (IsGirlJumpFlagged(state))
		{
			if (ShouldJump(state))
			{
				ExitCurrentState(state, rigid, coll);
				SetGirlJumpState(animCopy, state, rigid, targets);
			}
		}
		else if (IsGirlAttackFlagged(state))
		{
			if (ShouldAttack(state, animCopy))
			{
				ExitCurrentState(state, rigid, coll);
				SetGirlAttackState(animCopy, state, coll);
			}
		}
		else if (IsGirlCurrentlyJumping(state))
		{
			if (tf.position.y > lastPosition_.y)
			{
				ExitCurrentState(state, rigid, coll);
				SetGirlFallState(animCopy, state, rigid);
			}
		}
		else if (IsGirlCurrentlyFalling(state))
		{
			if (IsGirlOnGround(state))
			{
				ExitCurrentState(state, rigid, coll);
				SetGirlLandState(animCopy, state, rigid, coll);
			}
		}
		else if (IsGirlCurrentlyLanding(state))
		{
			if (IsGirlOnGround(state) && 
				IsGirlAtEndOfAnimationSeries(animCopy) &&
				TimeInAnimCrossesThreshold(animDeltas.landTime, 
										   animDeltas.landTimeEndMod))
			{
				ExitCurrentState(state, rigid, coll);
				SetGirlIdleState(animCopy, state);
			}
		}
		else if (IsGirlCurrentlyIdle(state))
		{
			if (GirlMoved(tf))
			{
				if (IsGirlOnGround(state))
				{
					ExitCurrentState(state, rigid, coll);
					SetGirlWalkState(animCopy, state);
				}
				else
				{
					ExitCurrentState(state, rigid, coll);
					SetGirlFallState(animCopy, state, rigid);
				}
			} 
		}
		else if (IsGirlCurrentlyWalking(state))
		{
			if (!IsGirlOnGround(state))
			{
				ExitCurrentState(state, rigid, coll);
				SetGirlFallState(animCopy, state, rigid);
			}
			else if (WalkVelocityXUnderStopThreshold(rigid) &&
					 !IsThumbstickEngaged(gc))
			{
				StopWalkVelocityX(rigid);
			}

			if (!GirlMoved(tf))
			{
				ExitCurrentState(state, rigid, coll);
				SetGirlIdleState(animCopy, state);
			}
		}
		else if (IsGirlCurrentlyAttacking(state))
		{
			if (IsGirlAtEndOfAnimationSeries(animCopy) &&				
				TimeInAnimCrossesThreshold(animDeltas.attackTime))
			{
				if (IsGirlOnGround(state))
				{
					ExitCurrentState(state, rigid, coll);
					SetGirlIdleState(animCopy, state);
				}
				else
				{
					ExitCurrentState(state, rigid, coll);
					SetGirlFallState(animCopy, state, rigid);
				}
			}
		}

		UpdateAnimations(tf, animCopy, state, animDeltas);
		UpdateSpriteFacingSide(tf, rend, state);

		if (SpriteChanged(animCopy, e))
		{
			UpdateForSpriteChange(animCopy, e);
		}

		DispatchStateEvents();

		FinalizeUpdates(tf, state);
	}


private:
	static bool IsGirlAtEndOfAnimationSeries(const SpriteAnimationComponent& anim)
	{
		return anim.index.current == anim.index.max;
	}
	bool TimeInAnimCrossesThreshold(float threshold, float mod = 0.0f) const
	{
		return timeInCurrentAnimFrame_ >= threshold + mod;
	}

	static bool IsGirlOnGround(const GirlState& state)
	{
		return state.collidingCategories[ObjectCategory::Ground] > 0;
	}
	static bool IsThumbstickEngaged(const GameControllerState& gc)
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
	static bool IsGirlJumpFlagged(const GirlState& state)
	{
		return state.jumpInitiated;
	}
	static bool IsGirlAttackFlagged(const GirlState& state)
	{
		return state.attackInitiated;
	}
	static bool IsGirlCurrentlyJumping(const GirlState& state)
	{
		return state.animation == GirlState::Animation::Jumping;
	}
	static bool IsGirlCurrentlyFalling(const GirlState& state)
	{
		return state.animation == GirlState::Animation::Falling;
	}
	static bool IsGirlCurrentlyIdle(const GirlState& state)
	{
		return state.animation == GirlState::Animation::Idle;
	}
	static bool IsGirlCurrentlyLanding(const GirlState& state)
	{
		return state.animation == GirlState::Animation::Landing;
	}
	static bool IsGirlCurrentlyWalking(const GirlState& state)
	{
		return state.animation == GirlState::Animation::Walking;
	}
	static bool IsGirlCurrentlyAttacking(const GirlState& state)
	{
		return state.animation == GirlState::Animation::Attacking;
	}

	void PushStateChangeEvent(const GirlState& state, GirlState::Animation newState)
	{
		if (eventBus_)
		{
			eventBus_->PushEvent(GirlStateChangeEvent{
				.lastState = state.animation,
				.newState = newState
			});

			shouldDispatchEvents_ = true;
		}
	}

	void SetGirlIdleState(SpriteAnimationComponent& anim, GirlState& state) 
	{
		anim.spriteSeriesName = "girl_idle";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Idle);

		state.animation = GirlState::Animation::Idle;
	}
	void SetGirlJumpState(SpriteAnimationComponent& anim, GirlState& state,
						  RigidBody& rigid, const MoveTargets& targets)
	{
		anim.spriteSeriesName = "girl_jump";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Jumping);

		state.animation = GirlState::Animation::Jumping;

		const float jumpImpulseY = ComputeJumpImpulseY(rigid, targets);

		rigid.forceRequests.impulses.emplace_back(
			Force{ .value = { 0.0f, -jumpImpulseY } });
	}
	void SetGirlWalkState(SpriteAnimationComponent& anim, GirlState& state)
	{
		anim.spriteSeriesName = "girl_walk";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Walking);

		state.animation = GirlState::Animation::Walking;
	}
	void SetGirlLandState(SpriteAnimationComponent& anim, GirlState& state,
						  RigidBody& rigid, Collider& collider)
	{
		anim.spriteSeriesName = "girl_land";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Landing);

		state.animation = GirlState::Animation::Landing;

		auto& body = WriteAccessor<B2Body>{}(rigid.body);
		body.SetLinearVelocity({ 0.0f, 0.0f });

		auto& shape = WriteAccessor<B2Shape>{}(collider.shape);
		shape.SetFriction(kGirlColliderLandingFriction);
	}
	static void ExitGirlLandState(Collider& collider)
	{
		auto& shape = WriteAccessor<B2Shape>{}(collider.shape);
		shape.SetFriction(kGirlColliderFriction);
	}

	void SetGirlFallState(SpriteAnimationComponent& anim, GirlState& state,
						  RigidBody& rigid)
	{
		anim.spriteSeriesName = "girl_fall";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Falling);

		state.animation = GirlState::Animation::Falling;

		auto& body = WriteAccessor<B2Body>{}(rigid.body);
		body.SetGravityScale(2.5f);
	}
	static void ExitGirlFallState(RigidBody& rigid)
	{
		auto& body = WriteAccessor<B2Body>{}(rigid.body);
		body.SetGravityScale(1.0f);
	}

	void SetGirlAttackState(SpriteAnimationComponent& anim, GirlState& state,
							Collider& collider)
	{
		anim.spriteSeriesName = "girl_attack";
		anim.index.current = 0;

		PushStateChangeEvent(state, GirlState::Animation::Attacking);

		state.animation = GirlState::Animation::Attacking;

		if (IsGirlOnGround(state))
		{
			auto& shape = WriteAccessor<B2Shape>{}(collider.shape);
			shape.SetFriction(kGirlColliderLandingFriction);
		}
	}
	static void ExitGirlAttackState(Collider& collider)
	{
		auto& shape = WriteAccessor<B2Shape>{}(collider.shape);
		shape.SetFriction(kGirlColliderFriction);
	}

	bool GirlMoved(const Transform& tf) const
	{
		return !(EqualsWithTolerance(tf.position.x, lastPosition_.x) &&
				 EqualsWithTolerance(tf.position.y, lastPosition_.y));
	}

	static bool WalkVelocityXUnderStopThreshold(RigidBody& rigid)
	{
		return std::abs(rigid.body.GetData().GetLinearVelocity().x) <
			   kGirlWalkStopVelocityX;
	}

	static void StopWalkVelocityX(RigidBody& rigid)
	{
		auto& body = WriteAccessor<B2Body>{}(rigid.body);
		body.SetLinearVelocity({
			0.0f,
			body.GetLinearVelocity().y
		});
	}

	void ExitCurrentState(GirlState& state, RigidBody& rigid, Collider& collider)
	{
		switch (state.animation)
		{
		case GirlState::Animation::Falling:
			ExitGirlFallState(rigid); break;
		case GirlState::Animation::Landing:
			ExitGirlLandState(collider); break;
		case GirlState::Animation::Attacking:
			ExitGirlAttackState(collider); break;
		}
	}

	void UpdateAnimations(const Transform& tf, SpriteAnimationComponent& anim,
						  GirlState& state, const AnimationDeltas& deltas) const
	{
		using Animation = GirlState::Animation;

		if (state.animation == Animation::Idle &&
			timeInCurrentAnimFrame_ >= deltas.idleTime)
		{
			assert(anim.spriteSeriesName == "girl_idle");

			++anim.index;
		}
		else if (state.animation == Animation::Walking)
		{
			assert(anim.spriteSeriesName == "girl_walk");
			
			if (distanceInCurrentAnimFrame_.x >= deltas.walkDeltaX)
			{
				++anim.index;
			}
		}
		else if (state.animation == Animation::Jumping)
		{
			assert(anim.spriteSeriesName == "girl_jump");

			if (distanceInCurrentAnimFrame_.y >= deltas.jumpDeltaY &&
				anim.index.current < anim.index.max)
			{
				++anim.index;
			}
		}
		else if (state.animation == Animation::Falling)
		{
			assert(anim.spriteSeriesName == "girl_fall");

			if (distanceInCurrentAnimFrame_.y >= deltas.fallDeltaY &&
				anim.index.current < anim.index.max)
			{
				++anim.index;
			}
		}
		else if (state.animation == Animation::Landing &&
				 timeInCurrentAnimFrame_ >= deltas.landTime &&
				 anim.index.current < anim.index.max)
		{ 	
			assert(anim.spriteSeriesName == "girl_land");

			++anim.index;
		}
		else if (state.animation == Animation::Attacking &&
				 timeInCurrentAnimFrame_ >= deltas.attackTime &&
				 anim.index.current < anim.index.max)
		{
			assert(anim.spriteSeriesName == "girl_attack");

			++anim.index;
		}
	}

	void UpdateSpriteFacingSide(Transform& tf, SpriteRenderableComponent& rend,
								GirlState& state) const
	{
		if (state.animation == GirlState::Animation::Landing)
		{
			return;
		}

		float xMoveDelta = lastPosition_.x - tf.position.x;

		rend.profile.flip = (tf.position.x < lastPosition_.x) ? SDL_FLIP_HORIZONTAL :
							(tf.position.x > lastPosition_.x) ? SDL_FLIP_NONE :
							rend.profile.flip;
	}

	void UpdateMemberDeltas(const Transform& tf, MoveTargets& targets, float dt)
	{
		distanceInCurrentAnimFrame_.x +=
			std::abs(tf.position.x - lastPosition_.x);
		distanceInCurrentAnimFrame_.y +=
			std::abs(tf.position.y - lastPosition_.y);

		timeInCurrentAnimFrame_ += dt;
		targets.dt = dt;
	}

	static bool ShouldAttack(const GirlState& state, const SpriteAnimationComponent& anim)
	{
		if (state.animation == GirlState::Animation::Attacking)
		{
			return false;
		}
		if (state.animation == GirlState::Animation::Landing)
		{
			return anim.index.current >= anim.index.max - 1;
		}

		return true;
	}

	//// TODO: implement this
	static bool ShouldJump(GirlState& state)
	{
		return !IsGirlCurrentlyJumping(state);
		//return IsGirlOnGround(state) &&
		//	   state.animation != GirlState::Animation::Attacking &&
		//	   state.animation != GirlState::Animation::Jumping &&
		//	   state.animation != GirlState::Animation::Falling;
	}

	static void HandleAxisMoveIntent(RigidBody& rigid, GirlState& state, 
									 MoveTargets& targets)
	{
		if (!state.axisMoveIntentX.has_value())
		{
			return;
		}
		if (IsGirlCurrentlyAttacking(state))
		{
			state.axisMoveIntentX.reset();
			return;
		}

		const float normedX = 
			static_cast<float>(*state.axisMoveIntentX) /
			static_cast<float>(GameController::kAxisMax);

		targets.targetVelX = normedX * targets.maxSpeed;

		float moveImpulseX = ComputeMoveImpulseX(rigid, state, targets);
		if (IsGirlCurrentlyLanding(state))
		{
			moveImpulseX /= 2.0f;
		}

		rigid.forceRequests.impulses.emplace_back(
			Force{ .value = { moveImpulseX, 0.0f } });
	}

	static SpriteAnimationComponent GetAnimComponentCopy(const Entity& girl)
	{
		return girl.GetComponent<SpriteAnimationComponent>();
	}

	static bool SpriteChanged(const SpriteAnimationComponent& animCopy, 
							  const Entity& girl)
	{
		return animCopy != girl.GetComponent<SpriteAnimationComponent>();
	}

	void UpdateForSpriteChange(SpriteAnimationComponent& animCopy,
							   Entity& girl)
	{
		girl.GetComponent<SpriteAnimationComponent>() = std::move(animCopy);
		timeInCurrentAnimFrame_ = 0.0f;
		distanceInCurrentAnimFrame_ = { 0.0f, 0.0f };
	}

	void DispatchStateEvents()
	{
		if (shouldDispatchEvents_ && eventBus_)
		{
			eventBus_->DispatchEvents();
		}
		shouldDispatchEvents_ = false;
	}

	void FinalizeUpdates(const Transform& tf, GirlState& state)
	{
		lastPosition_ = tf.position;
		lastState_ = state;

		state.jumpInitiated = false;
		state.attackInitiated = false;
		state.axisMoveIntentX.reset();
	}

	Entity_t girl_;
	SDL_FPoint lastPosition_ = { 0.0f, 0.0f };
	GirlState lastState_;
	float timeInCurrentAnimFrame_ = 0.0f;
	SDL_FPoint distanceInCurrentAnimFrame_ = { 0.0f, 0.0f };
	EventBus2* eventBus_ = nullptr;
	bool shouldDispatchEvents_ = false;
};


static Result<Void> RunPlatformerDemo(SceneFixture::SharedPtr& fixture)
{
	TRY(SetUpEnvironment(fixture));

	const auto [winCenterX, winCenterY] = 
		SDLite::Window().GetLocalCenter<SDL_FPoint>();

	TRY(MakeGirlEntity(fixture, {winCenterX, winCenterY - 60.0f}), girlEnt);

	fixture->RegisterSystem<GirlStateUpdater>(Phase::Input, 
		girlEnt.GetID(), fixture->GetEventBus());

	TRY(SetUpGirlStateReporter(girlEnt, fixture));

#if IMGUI_ENABLED

	assert(fixture->IsSystemRegistered<GuiSystem>());

	auto& guiSys = fixture->GetSystem<GuiSystem>();

	guiSys.AddWidget("Girl Physics Editor", [girlEnt] mutable {
		if (girlEnt.IsValid())
		{
			GirlPhysicsEditor::Draw(girlEnt);
		}
	});

#endif

	return fixture->RunGameLoop();
}


















} // test