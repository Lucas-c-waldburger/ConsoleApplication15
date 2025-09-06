#pragma once
#include "../Resources.h"
#include "../../ecs/ECS.h"
#include "../../components/driver/SpriteAnimationDriver.h"
#include "../../components/driver/EntityStateDriver.h"
#include "../../components/EntityStateComponent.h"
#include "../callbacks/AnimationCallbacks.h"
#include "../../state/EntityState.h"
#include "../../components/util/SpriteAnimationUtils.h"

namespace test {

namespace {

bool UpdateFaceDirection(const GameControllerState& controller, Renderable& renderable)
{
	auto axisValue = controller.inputs[GameControllerInputSource::LeftStickAxis].value.axis;

	if (axisValue.x < 0.0f)
	{
		renderable.profile.flip = SDL_FLIP_HORIZONTAL;
		return true;
	}
	else if (axisValue.x > 0.0f)
	{
		renderable.profile.flip = SDL_FLIP_NONE;
		return true;
	}
	
	return false;
}

void HandleKnightAnimationStateExit(Entity& entity)
{
	if (entity.HasComponent<SpriteAnimations>())
	{
		auto& animations = entity.GetComponent<SpriteAnimations>();

		bool needsUpdate = SetCurrentSpriteSeries(animations, "");
		if (needsUpdate)
		{
			entity.AddComponent<NeedsUpdate>().components |= SpriteAnimations::componentBit;
		}
	}
}

enum class TrackedValueType
{
	ByTime,
	ByTravelDistance
};

void HandleKnightAnimationStateEnter(Entity& entity, std::string_view stateName,
									 TrackedValueType trackedType, float thresholdValue)
{
	if (!entity.HasComponent<SpriteAnimations>())
	{
		LOG_ERROR_FMT("Entity did not have required components for {} state", stateName);
		return;
	}

	auto& animations = entity.GetComponent<SpriteAnimations>();

	assert(animations.current != stateName);

	if (!SetCurrentSpriteSeries(animations, stateName))
	{
		LOG_ERROR_FMT("Sprite series change to {} animation failed", stateName);
		return;
	}

	auto& series = animations.table[stateName];

	series.index = 0;
	series.spriteRange.min = 0;
	series.spriteRange.max = (series.spritePlots.size() > 0) ? series.spritePlots.size() - 1 : 0;

	series.seriesMetrics.time.reset();
	series.seriesMetrics.distance.reset();
	ThresholdTracker<float> tracker{ .threshold = thresholdValue };

	if (trackedType == TrackedValueType::ByTime)
	{
		series.seriesMetrics.time = tracker;
	}
	else
	{
		series.seriesMetrics.distance = tracker;
	}

	animations.current = std::string{ stateName };

	entity.AddComponent<NeedsUpdate>().components |= SpriteAnimations::componentBit;
}

//void SwitchState(SpriteAnimations& animations, std::string_view newSeriesName)
//{
//	assert(animations.current != newSeriesName);
//
//	if (!SetCurrentSpriteSeries(animations, newSeriesName))
//	{
//		LOG_ERROR("Sprite series change to idle animation failed");
//		return;
//	}
//
//	auto& newSeries = animations.table[newSeriesName];
//
//	idleSeries.index = 0;
//	idleSeries.seriesMetrics.time = ThresholdTracker<float>{ .threshold = 0.25f };
//	idleSeries.seriesMetrics.distance.reset();
//
//	animations.current = std::string{ kIdleSeriesName };
//}

} // detail
	  
constexpr float kJumpVelocity = 5.0f;

constexpr float TimeToJumpApex(float impulseY, float mass, SDL_FPoint gravity)
{
	float velocityY = impulseY / mass;

	if (gravity.y <= 0.0f || velocityY >= 0.0f)
	{
		return 0.0f;
	}

	return -velocityY / gravity.y;
}

auto AdvanceJumpAnimation()
{
	return [](Entity& entity, events::TimerFired& ev) -> ReturnSignal
	{
		auto relations = entity.GetRelations();
		if (!relations.IsChildOf(ev.producer))
		{
			return ReturnSignal::KeepObserving;
		}

		auto owner = ECS::GetEntityByID(ev.producer);
		assert(owner.IsValid());

		auto animationDriver = SpriteAnimationDriver::GetInstance(owner);
		if (!animationDriver.Success())
		{
			LOG_ERROR(animationDriver.GetError());
			return ReturnSignal::StopObserving;
		}

		animationDriver->AdvanceCurrentSeries();

		return ReturnSignal::StopObserving;
	};
}

void MakeChildJumpTimer(Entity& entity, float elapsed)
{
	//assert(entity.IsValid());

	//auto relations = entity.GetRelations();
	//assert(!relations.IsChild());

	//auto child = relations.AddChild();

	//child.AddComponent(Timer{
	//	.elapsed = elapsed,
	//	.flags = Timer::Flag::Active
	//}); 

	//auto& evCallbacks = child.AddComponent<EventCallbacks>();

	//auto& timerCallback = evCallbacks.table[events::TimerFired::eventType];
	//timerCallback = STR(AdvanceJumpAnimation);
}

auto TriggerJumpState()
{
	return [](Entity& entity, const events::GameControllerInput& ev) -> ReturnSignal
	{
		//auto driver = EntityStateDriver::GetInstance(entity);
		//if (!driver.Success())
		//{
		//	LOG_ERROR(driver.GetError());
		//	return ReturnSignal::KeepObserving;
		//}

		//auto result = driver->ChangeState(kJumpStateName);
		//if (!result.Success())
		//{
		//	LOG_ERROR(result.GetError());
		//	return ReturnSignal::KeepObserving;
		//}

		return ReturnSignal::StopObserving;
	};
}

auto OnJumpStateEnter()
{
	return [](Entity& entity) -> void  
	{
		// animation
		auto animationDriver = SpriteAnimationDriver::GetInstance(entity);
		if (!animationDriver.Success())
		{
			LOG_ERROR(animationDriver.GetError());
			return;
		}

		bool seriesSet = animationDriver->SetCurrentSeries(kJumpSeriesName);
		assert(seriesSet);

		bool eventEnabled = animationDriver->EnableCurrentSeriesEvents<events::SpriteIndexChange>();
		assert(eventEnabled);

		// physics
		assert(entity.HasComponent<RigidBody>());
		auto& rigidBody = entity.GetComponent<RigidBody>();

		float mass = rigidBody.body.GetData().GetMass();
		SDL_FPoint gravity = rigidBody.body.GetData().GetEffectiveGravity();
		float impulseY = -(mass * kJumpVelocity);
		
		rigidBody.forceRequests.impulses.emplace_back(Force{
			.value = { 0.0f, impulseY }
		});

		float timeToApex = TimeToJumpApex(impulseY, mass, gravity);

		size_t seriesSize = animationDriver->GetCurrentSeriesSize();
		assert(seriesSize > 0);

		float timePerStep = timeToApex / static_cast<float>(seriesSize);

		for (size_t i = 1; i <= seriesSize; i++)
		{
			MakeChildJumpTimer(entity, timePerStep * i);
		}

		// callbacks
		assert(entity.HasComponent<GameControllerInputCallbacks>());
		auto& inputCallbacks = entity.GetComponent<GameControllerInputCallbacks>();

		inputCallbacks.table.erase(GameControllerInputSource::A);
	};
}

auto OnJumpStateExit()
{
	return [](Entity& entity) -> void
	{
		assert(entity.HasComponent<EventCallbacks>());
		auto& evCallbacks = entity.GetComponent<EventCallbacks>();

		//evCallbacks.table[events::SpriteIndexChange::eventType] = {};
	};
}

auto OnWalkStateEnter()
{
	return [](Entity& entity) -> void 
	{
		//auto animationDriver = SpriteAnimationDriver::GetInstance(entity);
		//if (!animationDriver.Success())
		//{
		//	LOG_ERROR(animationDriver.GetError());
		//	return;
		//}

		//bool seriesSet = animationDriver->SetCurrentSeries(kWalkSeriesName);
		//assert(seriesSet);

		//assert(entity.HasComponent<GameControllerInputCallbacks>());
		//auto& inputCallbacks = entity.GetComponent<GameControllerInputCallbacks>();

		//using Source = GameControllerInputSource;

		//inputCallbacks.table[Source::A] = STR(TriggerJumpState);
		//inputCallbacks.table[Source::RightStickAxis] = STR(ApplyAxisInputToForce);
	};
}

auto OnWalkStateExit()
{
	return [](Entity& entity) -> void
		{

		};
}

//class KnightWalkState : public EntityState
//{
//public:
//	~KnightWalkState() override = default;
//
//	void OnEnter(Entity& entity)
//	{
//		if (!entity.HasComponents<GameControllerState, SpriteAnimations, Renderable>())
//		{
//			LOG_ERROR("Entity did not have required components for walk state");
//			return;
//		}
//
//		auto [controller, animations, renderable] =
//			entity.GetComponents<GameControllerState, SpriteAnimations, Renderable>();
//
//		auto it = animations.table.find(std::string{ kWalkSeriesName });
//		if (it == animations.table.end())
//		{
//			LOG_ERROR("Entity did not have walk sprite series in animation table");
//			return;
//		}
//
//		it->second.index = 0;
//		animations.current = std::string{ kWalkSeriesName };
//		
//
//
//	}
//
//	void OnExit(Entity& entity);
//
//	void OnUpdate(Entity& entity)
//	{
//		if (!entity.HasComponents<GameControllerState, SpriteAnimations, Renderable>())
//		{
//			LOG_ERROR("Entity did not have required components for walk state update");
//			return;
//		}
//
//		auto [controller, animations, renderable] =
//			entity.GetComponents<GameControllerState, SpriteAnimations, Renderable>();
//
//		auto axisValue = controller.inputs[GameControllerInputSource::RightStickAxis].value.axis;
//
//		renderable.profile.flip = (axisValue.x < 0.0f) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
//	}
//
//private:
//	Direction currentFace_ = Direction::W;
//};

//enum class StateResolutionPhase
//{
//	Physics,
//	Rendering
//};

class KnightWalkState;
class KnightIdleState;
class KnightLookUpState;
class KnightLookDownState;
class KnightJumpState;


class KnightWalkState
{
public:
	static constexpr float kFrameDistance = 20.0f;

	static void OnEnter(Entity& entity)
	{
		HandleKnightAnimationStateEnter(entity, kWalkSeriesName,
										TrackedValueType::ByTravelDistance, kFrameDistance);
	}

	static void OnExit(Entity& entity)
	{
		HandleKnightAnimationStateExit(entity);
	}

	static void OnUpdate(Entity& entity, float)
	{
		if (!entity.HasComponents<EntityStateComponent, GameControllerState, 
								  SpriteAnimations, Renderable>())
		{
			LOG_ERROR("Entity did not have required components for idle state update");
			return;
		}

		auto [stateComponent, controller, animations, renderable] =
			entity.GetComponents<EntityStateComponent, GameControllerState, 
								 SpriteAnimations, Renderable>();

		assert(stateComponent.stateID.value == EntityState<KnightIdleState>::GetStateID());

		using enum GameControllerInputSource;

		auto pressedA = controller.inputs[A].state == InputState::Pressed;
		if (pressedA)
		{
			stateComponent.stateID.requested = EntityState<KnightJumpState>::GetStateID();
			return;
		}

		auto axisValue = controller.inputs[LeftStickAxis].value.axis;

		if (axisValue.x < 0.0f)
		{
			renderable.profile.flip = SDL_FLIP_HORIZONTAL;
		}
		else if (axisValue.x > 0.0f)
		{
			renderable.profile.flip = SDL_FLIP_NONE;
		}
		else
		{
			stateComponent.stateID.requested = EntityState<KnightIdleState>::GetStateID();
			return;
		}

		assert(animations.current == std::string{ kWalkSeriesName });
		auto walkIt = animations.table.find(kWalkSeriesName);
		assert(walkIt != animations.table.end());

		auto& walkSeries = walkIt->second;

		assert(walkSeries.seriesMetrics.distance.has_value());
		auto& travelDistance = *walkSeries.seriesMetrics.distance;

		if (travelDistance.accumulated >= travelDistance.threshold)
		{
			AdvanceSpriteSeries(walkSeries);

			entity.AddComponent<NeedsUpdate>().components |= SpriteAnimations::componentBit;

			travelDistance.accumulated = 0.0f;
		}
	}
};


// IDLE STATE
class KnightIdleState
{
public:
	static constexpr float kFrameDuration = 0.25f;

	static void OnEnter(Entity& entity)
	{
		HandleKnightAnimationStateEnter(entity, kIdleSeriesName,
										TrackedValueType::ByTime, kFrameDuration);
	}

	static void OnExit(Entity& entity)
	{
		HandleKnightAnimationStateExit(entity);
	}

	static void OnUpdate(Entity& entity, float delta)
	{
		if (!entity.HasComponents<EntityStateComponent, GameControllerState, SpriteAnimations>())
		{
			LOG_ERROR("Entity did not have required components for idle state update");
			return;
		}

		auto [stateComponent, controller, animations] = 
			entity.GetComponents<EntityStateComponent, GameControllerState, SpriteAnimations>();

		assert(stateComponent.stateID.value == EntityState<KnightIdleState>::GetStateID());

		using enum GameControllerInputSource;

		bool pressedA = controller.inputs[A].state == InputState::Pressed;
		if (pressedA)
		{
			stateComponent.stateID.requested = EntityState<KnightJumpState>::GetStateID();
			return;
		}

		bool movedLeftStick = controller.inputs[LeftStickAxis].state == InputState::Pressed;
		if (movedLeftStick)
		{
			auto axisValue = controller.inputs[LeftStickAxis].value.axis;

			if (axisValue.y > std::abs(axisValue.x)) 
			{
				stateComponent.stateID.requested = EntityState<KnightLookUpState>::GetStateID();
			}
			else
			{
				stateComponent.stateID.requested = EntityState<KnightWalkState>::GetStateID();
			}
		
			return;
		}

		assert(animations.current == std::string{ kIdleSeriesName });
		auto idleIt = animations.table.find(kIdleSeriesName);
		assert(idleIt != animations.table.end());

		auto& idleSeries = idleIt->second;

		assert(idleSeries.seriesMetrics.time.has_value());
		auto& time = *idleSeries.seriesMetrics.time;

		time.accumulated += delta;
		if (time.accumulated >= time.threshold)
		{
			AdvanceSpriteSeries(idleSeries);

			entity.AddComponent<NeedsUpdate>().components |= SpriteAnimations::componentBit;

			time.accumulated = 0.0f;
		}
	}
};


class KnightLookUpState
{
public:
	static constexpr float kFrameDuration = 0.25f;
	static constexpr size_t kLoopSectionStartIndex = 1;

	static void OnEnter(Entity& entity)
	{
		HandleKnightAnimationStateEnter(entity, kLookUpSeriesName, 
										TrackedValueType::ByTime, kFrameDuration);
				
	}

	static void OnUpdate(Entity& entity, float delta) 
	{
		if (!entity.HasComponents<EntityStateComponent, GameControllerState, SpriteAnimations>())
		{
			LOG_ERROR("Entity did not have required components for look-up state update");
			return;
		}

		auto [stateComponent, controller, animations] =
			entity.GetComponents<EntityStateComponent, GameControllerState, SpriteAnimations>();

		assert(stateComponent.stateID.value == EntityState<KnightLookUpState>::GetStateID());

		using enum GameControllerInputSource;

		auto pressedA = controller.inputs[A].state == InputState::Pressed;
		if (pressedA)
		{
			stateComponent.stateID.requested = EntityState<KnightJumpState>::GetStateID();
			return;
		}

		auto leftStickEngaged = controller.inputs[LeftStickAxis].state == InputState::Pressed ||
								 controller.inputs[LeftStickAxis].state == InputState::Held;
		if (leftStickEngaged)
		{
			auto axisValue = controller.inputs[LeftStickAxis].value.axis;

			if (std::abs(axisValue.x) > axisValue.y)
			{
				stateComponent.stateID.requested = EntityState<KnightWalkState>::GetStateID();
				return;
			}
		}
		else // released or none
		{
			stateComponent.stateID.requested = EntityState<KnightLookDownState>::GetStateID();
			return;
		}

		assert(animations.current == std::string{ kLookUpSeriesName });
		auto lookupIt = animations.table.find(kLookUpSeriesName);
		assert(lookupIt != animations.table.end());

		auto& lookupSeries = lookupIt->second;

		assert(lookupSeries.seriesMetrics.time.has_value());
		auto& time = *lookupSeries.seriesMetrics.time;

		time.accumulated += delta;
		if (time.accumulated >= time.threshold)
		{
			AdvanceSpriteSeries(lookupSeries);

			if (lookupSeries.index > 0)
			{
				lookupSeries.spriteRange.min = kLoopSectionStartIndex;
			}

			entity.AddComponent<NeedsUpdate>().components |= SpriteAnimations::componentBit;

			time.accumulated = 0.0f;
		}
	}

private:
};

} // test