#include "GameLoopSystem.h"
#include "../events/EventDataTypeList.h"
#include "../events/EventBus.h"

void GameLoopSystem::UpdateLoopStepStart(EventBus2& bus)
{
	counter_.Update();

	bus.PushEvent(events::GameLoopStepStart{});
	bus.DispatchEvents();

	//EventBus::PushEvent(events::GameLoopStepStart{});
	//EventBus::DispatchEvents<events::GameLoopStepStart>();
}

void GameLoopSystem::UpdateLoopStepRender(EventBus2& bus)
{
	bus.PushEvent(events::GameLoopStepRender{});
	bus.DispatchEvents();

	//EventBus::PushEvent(events::GameLoopStepRender{});
	//EventBus::DispatchEvents<events::GameLoopStepRender>();
}

void GameLoopSystem::UpdateLoopStepEnd(EventBus2& bus)
{
	bus.PushEvent(events::GameLoopStepEnd{});
	bus.DispatchEvents();

	bus.DiscardEvents();

	//EventBus::PushEvent(events::GameLoopStepEnd{});
	//EventBus::DispatchEvents<events::GameLoopStepEnd>();

	//EventBus::ClearEvents();
}

float GameLoopSystem::GetDeltaTime() const
{
	return counter_.GetDelta();
}
