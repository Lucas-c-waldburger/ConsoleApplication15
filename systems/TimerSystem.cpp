#include "TimerSystem.h"
#include "../ecs/Ecs.h"
#include "../events/EventBus.h"
#include "../events/data/TimeEvents.h"
#include "../core/CommonEntityMethods.h"

void TimerSystem::Update(float delta, EventBus2& bus)
{
	auto entities = ECS::GetAllEntitiesWith<Timer>();

	for (auto& entity : entities)
	{
		auto& timer = entity.GetComponent<Timer>();

		if ((timer.flags & Timer::Flag::Active) == 0)
		{
			continue;
		}

		timer.elapsed += delta;
		if (timer.elapsed < timer.duration)
		{
			continue;
		}
		
		if (EntityShouldProduceEvent<events::TimerFired>(entity))
		{
			bus.PushEvent(events::TimerFired{
				.producer = entity.GetID(),
				.duration = timer.duration
			});
		}

		timer.elapsed = 0.0f;

		if ((timer.flags & Timer::Flag::Repeating) == 0)
		{
			if (timer.flags & Timer::Flag::RemoveOnExpiry)
			{
				entity.RemoveComponent<Timer>();
			}
			else
			{
				timer.flags &= ~(Timer::Flag::Active);
			}
		}
	}

	bus.DispatchEvents();
}
