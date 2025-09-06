#include "GameLoopSystem.h"
#include "../events/EventDataTypeList.h"
#include "../events/EventBus.h"

void GameLoopSystem::UpdateLoopStepStart()
{
	counter_.Update();

	EventBus::PushEvent(events::GameLoopStepStart{});
	EventBus::DispatchEvents<events::GameLoopStepStart>();
}

void GameLoopSystem::UpdateLoopStepRender()
{
	EventBus::PushEvent(events::GameLoopStepRender{});
	EventBus::DispatchEvents<events::GameLoopStepRender>();
}

void GameLoopSystem::UpdateLoopStepEnd()
{
	EventBus::PushEvent(events::GameLoopStepEnd{});
	EventBus::DispatchEvents<events::GameLoopStepEnd>();

	EventBus::ClearEvents();
}

float GameLoopSystem::GetDeltaTime() const
{
	return counter_.GetDelta();
}
