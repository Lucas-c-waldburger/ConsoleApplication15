#include "TimerSystem.h"
#include "../ecs/Ecs.h"
#include "../events/data/TimeEvents.h"

namespace {

void HandleTimerExpiry(Timer& timer, Entity& e)
{
	if (timer.numRepeats == 0)
	{
		if (timer.flags & Timer::Flag::RemoveOnExpiry)
		{
			e.RemoveComponent<Timer>();
		}
		else
		{
			timer.flags &= ~(Timer::Flag::Active);
		}
	}
}

} // unnamed

void TimerSystem::Update(float delta, EventBus& bus)
{
	auto entities = ECS::GetAllEntitiesWith<Timer>();

	for (auto& entity : entities)
	{
		auto& timer = entity.GetComponent<Timer>();

		if ((timer.flags & Timer::Flag::Active) == 0)
		{
			continue;
		}

		HandleTimerExpiry(timer, entity);

		timer.elapsed += delta;
		if (timer.elapsed < timer.duration)
		{
			continue;
		}
		
		if (entity.ShouldProduceEvent<events::TimerFired>())
		{
			events::TimerFired ev{};
			ev.entity<0>() = entity.GetID();

			bus.PushEvent(std::move(ev));
		}

		timer.elapsed = 0.0f;
		timer.numRepeats = (timer.numRepeats > 0)
			? timer.numRepeats - 1
			: timer.numRepeats;

		HandleTimerExpiry(timer, entity);
	}

	bus.DispatchEvents();
}
