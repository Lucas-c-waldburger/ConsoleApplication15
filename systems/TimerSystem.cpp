#include "TimerSystem.h"
#include "../ecs/Ecs.h"
#include "../events/EventBus.h"
#include "../events/data/TimeEvents.h"

namespace {

//constexpr bool IsRepeating(const Timer& timer)
//{
//
//}

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
			firedEvent.parent = parent.GetID();
		}
	}

	EventBus::PushEvent(std::move(firedEvent));
}

} // unnamed

void TimerSystem::Update(float delta)
{
	auto entities = ECS::GetAllEntitiesWith<Timer>([](const Timer& timer) {
			return timer.flags & Timer::Flag::Active;
		});

	for (auto& entity : entities)
	{
		auto& timer = entity.GetComponent<Timer>();

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
