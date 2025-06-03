#pragma once
#include "System.h"
#include "../events/handler/GameControllerEventHandler.h"

class SDLInputSystem : public System
{
public:
	bool Update();

private:
	SDL_Event sdlEvent_;
	GameControllerEventHandler gameControllerHandler_;
};

