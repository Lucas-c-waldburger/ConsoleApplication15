#pragma once
#include "System.h"


class EventSystem : public System
{
public:
	void DispatchEvents();
	void ResetEventBus();

private:
};