#pragma once
#include "../../callbacks/StateTransitionCallbackRegistry.h"
#include "../../components/driver/SpriteAnimationDriver.h"
#include "../../components/driver/EntityStateDriver.h"
#include "../../components/EntityStateComponent.h"
#include "../callbacks/AnimationCallbacks.h"

namespace test {

constexpr std::string_view kWalkSeriesName = "walk";
constexpr std::string_view kJumpSeriesName = "jump";
constexpr std::string_view kFallSeriesName = "fall";

constexpr std::string_view kWalkStateName = "walk_state";
constexpr std::string_view kJumpStateName = "jump_state";
constexpr std::string_view kFallStateName = "fall_state";

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
		if (!relations.IsChildOf(ev.owner))
		{
			return ReturnSignal::KeepObserving;
		}

		auto owner = ECS::GetEntityByID(ev.owner);
		assert(owner.IsValid());

		auto animationDriver = SpriteAnimationDriver::GetInstance(owner);
		if (!animationDriver.Success())
		{
			LOG_ERROR(animationDriver.GetError());
			return ReturnSignal::StopObserving;
		}

		animationDriver->Step();

		return ReturnSignal::StopObserving;
	};
}

void MakeChildJumpTimer(Entity& entity, float time)
{
	assert(entity.IsValid());

	auto relations = entity.GetRelations();
	assert(!relations.IsChild());

	auto child = relations.AddChild();

	child.AddComponent(Timer{
		.time = time,
		.flags = Timer::Flag::Active
	}); 

	auto& evCallbacks = child.AddComponent<EventCallbacks>();

	auto& timerCallback = evCallbacks.table[events::TimerFired::eventType];
	timerCallback.name = STR(AdvanceJumpAnimation);
	timerCallback.fn = nullptr;
}

auto TriggerJumpState()
{
	return [](Entity& entity, const events::GameControllerInput& ev) -> ReturnSignal
	{
		auto driver = EntityStateDriver::GetInstance(entity);
		if (!driver.Success())
		{
			LOG_ERROR(driver.GetError());
			return ReturnSignal::KeepObserving;
		}

		auto result = driver->ChangeState(kJumpStateName);
		if (!result.Success())
		{
			LOG_ERROR(result.GetError());
			return ReturnSignal::KeepObserving;
		}

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

		evCallbacks.table[events::SpriteIndexChange::eventType] = {};
	};
}

auto OnWalkStateEnter()
{
	return [](Entity& entity) -> void 
	{
		auto animationDriver = SpriteAnimationDriver::GetInstance(entity);
		if (!animationDriver.Success())
		{
			LOG_ERROR(animationDriver.GetError());
			return;
		}

		bool seriesSet = animationDriver->SetCurrentSeries(kWalkSeriesName);
		assert(seriesSet);

		assert(entity.HasComponent<GameControllerInputCallbacks>());
		auto& inputCallbacks = entity.GetComponent<GameControllerInputCallbacks>();

		using Source = GameControllerInputSource;

		inputCallbacks.table[Source::A].name = STR(TriggerJumpState);
		inputCallbacks.table[Source::RightStickAxis].name = STR(ApplyAxisInputToForce);
	};
}

} // test