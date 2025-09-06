#pragma once
#include "System.h"
#include "../core/Counter.h"

class GameLoopSystem : public System
{
public:
	void UpdateLoopStepStart();
	void UpdateLoopStepRender();
	void UpdateLoopStepEnd();

	float GetDeltaTime() const;

private:
	Counter counter_;
};
