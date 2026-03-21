#pragma once
#include "../IEventData.h"
#include "../EventConcepts.h"
#include "../../ecs/EntityT.h"
#include "../../components/TimerComponent.h"

namespace events {

//struct TimerFired : IEventData<TimerFired> 
//{
//	Entity_t producer = kInvalidEntity;
//	float duration = 0.0f;
//};

struct TimerFired : IEventData<TimerFired>, EntityParticipants<1>
{
};

} // events