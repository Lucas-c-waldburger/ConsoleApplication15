#pragma once
#include "System.h"
#include "../core/Counter.h"
#include "../events/EventBus2.h"

class GameLoopSystem : public System
{
public:
	void UpdateLoopStepStart(EventBus& bus);
	void UpdateLoopStepRender(EventBus& bus);
	void UpdateLoopStepEnd(EventBus& bus);

	const Counter& GetCounter() const { return counter_; }

	float GetDeltaTime() const;

private:
	Counter counter_;
};
