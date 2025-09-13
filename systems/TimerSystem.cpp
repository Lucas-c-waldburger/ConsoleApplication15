#include "TimerSystem.h"
#include "../ecs/Ecs.h"
#include "../events/EventBus.h"
#include "../events/data/TimeEvents.h"

namespace {

//void PushTimerFiredEvent(Entity& entity, const Timer& timer)
//{
//	events::TimerFired firedEvent{
//		.producer = entity.GetID(),
//		.duration = timer.duration
//	};
//
//	//auto relations = entity.GetRelations();
//	//if (relations.IsChild())
//	//{
//	//	auto parent = relations.GetParent();
//	//	if (parent.IsValid())
//	//	{
//	//		firedEvent.producer = parent.GetID();
//	//	}
//	//}
//
//	EventBus::PushEvent(std::move(firedEvent));
//}

} // unnamed

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
		
		bus.PushEvent(events::TimerFired{
			.producer = entity.GetID(),
			.duration = timer.duration
		});
		//PushTimerFiredEvent(entity, timer);

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
	//EventBus::DispatchEvents<events::TimerFired>();
}
