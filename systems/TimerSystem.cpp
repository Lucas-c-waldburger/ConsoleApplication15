#include "TimerSystem.h"
#include "../ecs/Ecs.h"
#include "../events/EventBus.h"
#include "../events/data/TimeEvents.h"

namespace {

void PushTimerFiredEvent(Entity& entity, const Timer& timer)
{
	events::TimerFired firedEvent{
		.duration = timer.duration
	};

	auto relations = entity.GetRelations();
	if (relations.IsChild())
	{
		auto parent = relations.GetParent();
		if (parent.IsValid())
		{
			firedEvent.owner = parent.GetID();
		}
	}

	EventBus::PushEvent(std::move(firedEvent));
}

} // unnamed

void TimerSystem::Update(float delta)
{
	auto entities = ECS::GetAllEntitiesWith<Timer>();

	for (auto& entity : entities)
	{
		auto& timer = entity.GetComponent<Timer>();

		if ((timer.flags & Timer::Flag::Active) == 0)
		{
			continue;
		}

		timer.time += delta;
		if (timer.time < timer.duration)
		{
			continue;
		}
		
		PushTimerFiredEvent(entity, timer);

		timer.time = 0.0f;

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

	EventBus::DispatchEvents<events::TimerFired>();
}
