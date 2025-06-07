#pragma once
#include "System.h"
#include "EventSystem.h"
#include "../events/handler/GameControllerEventHandler.h"

class SDLInputSystem : public System
{
public:
	bool Update(EventSystem& eventSystem);

private:
	GameControllerEventHandler gameControllerHandler_;
};

