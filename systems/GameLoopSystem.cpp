#include "GameLoopSystem.h"
#include "../events/EventDataTypeList.h"

void GameLoopSystem::UpdateLoopStepStart(EventBus& bus)
{
	counter_.Update();

	bus.PushEvent(events::GameLoopStepStart{});
	bus.DispatchEvents();
}

void GameLoopSystem::UpdateLoopStepRender(EventBus& bus)
{
	bus.PushEvent(events::GameLoopStepRender{});
	bus.DispatchEvents();
}

void GameLoopSystem::UpdateLoopStepEnd(EventBus& bus)
{
	bus.PushEvent(events::GameLoopStepEnd{});
	bus.DispatchEvents();

	bus.DiscardEvents();
}

float GameLoopSystem::GetDeltaTime() const
{
	return counter_.GetDelta();
}
