#include "GameLoopSystem.h"
#include "../events/EventDataTypeList.h"
#include "../events/EventBus.h"

void GameLoopSystem::UpdateLoopStepStart(EventBus2& bus)
{
	counter_.Update();

	bus.PushEvent(events::GameLoopStepStart{});
	bus.DispatchEvents();
}

void GameLoopSystem::UpdateLoopStepRender(EventBus2& bus)
{
	bus.PushEvent(events::GameLoopStepRender{});
	bus.DispatchEvents();
}

void GameLoopSystem::UpdateLoopStepEnd(EventBus2& bus)
{
	bus.PushEvent(events::GameLoopStepEnd{});
	bus.DispatchEvents();

	bus.DiscardEvents();
}

float GameLoopSystem::GetDeltaTime() const
{
	return counter_.GetDelta();
}
